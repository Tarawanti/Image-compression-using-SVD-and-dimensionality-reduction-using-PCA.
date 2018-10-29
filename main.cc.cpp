 
#include "stdafx.h"

#include "eigen3/Eigen/SVD"
#include "eigen3/Eigen/Eigen"

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <string>

#include <sstream>
#include <vector>
using namespace std;

// procedures for converting from ascii to binary and back
void asciiToBinary(std::string file); // file - filename of ascii pgm file
void binaryToAscii(std::string file); // file - filename of binary pgm file

									  // procedure for creating files with header and matrices for svd
void asciiToSvd(std::string file); // file - filename of ascii pgm file

								   // procedure for svd compressing
								   // header - filename with header(width, height and greyscale)
								   // svd - filename with matrices for svd
								   // k - rank of the approximation
void svdCompress(std::string header, std::string svd, int k);

// procedure for svd decompressing
// file - name of the compressed file
void svdDecompress(std::string file);

// ----------------------------------------------------------------------------------------

void asciiToBinary(std::string file)
{
	// We'll read ascii pgm file line by line
	std::string line;

	// file stream for reading 
	std::ifstream inputFile(file);

	// Checking for the success of opening
	if (!inputFile)
	{
		std::cerr << "Could not open file " << file << "\n";
		return;
	}

	// Special variable for reading width, height and grayscale from file stream "inputFile" to
	short int value;

	// Variable for line number
	int lineNum = 0;

	// The buffer for all pixels of pgm file (the header is not included)
	std::vector<unsigned char> buffer;

	// Variables for width, height and grayscale
	short int xSize, ySize, maxg;

	// Variable to distinguish what we're reading from the file stream right now (width, height or grayscale)
	int step = 0;

	// Reading ascii pgm file line by line
	while (std::getline(inputFile, line))
	{
		// Now the 'line' variable contains a current line

		// Creating a stream for the current line parsing
		std::istringstream iss(line);

		// And we'll read only numbers (because 'value' has short int type)
		// P2 in the pgm file header will be skipped because it's not a number
		while (iss >> value)
		{
			// So we need to get width(xSize), height(ySize) and grayscale(maxg) - to pass 3 steps
			if (step < 3)
			{
				switch (step)
				{
					// the first step(with index 0) - getting width from file stream
				case 0:
					xSize = value;
					break;
					// the seocnd step(with index 1) - getting height from file stream
				case 1:
					ySize = value;
					break;
					// and getting grayscale value 
				case 2:
					maxg = value;
					break;
				default:
					break;
				};

				// Incrementing 'step' if it's still less then 3
				++step;

			}
			else
			{
				// so here we have step >=3 (we've already gotten info from pgm header)
				// so simply pushing pixel gray values to the vector we created
				buffer.push_back(value);
			}
		}
	}

	// closing pgm ascii file descriptor
	inputFile.close();


	// Write to output file image_b.pgm

	// Creating the output filename
	// Firstly finding input filename without its extension
	int lastIndex = file.find_last_of(".");

	// and adding _b.pgm to get output filename we want
	std::string oFile = file.substr(0, lastIndex) + "_b.pgm";

	// Creating a stream for writing to the output file
	std::ofstream outputFile(oFile, std::ofstream::binary);

	// Writing the pgm header for binary format (P5 instead of P2)
	outputFile << "P5" << " "
		<< xSize << " "		// writing image width
		<< ySize << " "		// writing image height
		<< maxg << "\n";	// writing  grayscale value


	for (unsigned int i = 0; i < buffer.size(); ++i)
		outputFile.write((char*)&buffer[i], sizeof(buffer[i]));
	// buffer - the full vector with gray values
	// buffer[i] - the each element (it has type unsigned char) of the vector
	// &buffer[i] - the pointer to the each element 
	// we should do (char*)&buffer[i] for casting. Because the first parameter of the function write has type char* and not unsigned char 
	// so we tell to the proc write to take the each element of the vector and we say that we should take sizeof(buffer[i]) bytes from the pointer &buffer[i]
	// for getting the value of the element of the vector

	// Closing output file descriptor
	outputFile.close();

}

void binaryToAscii(std::string file)
{
	// Creating a stream for reading pgm binary file
	// std::ifstream::ate flag means that we want to set current position of the file to its end
	std::ifstream inputFile(file, std::ifstream::binary | std::ifstream::ate);

	// Checking for the success of opening 
	if (!inputFile)
	{
		std::cerr << "Could not open file " << file << "\n";
		return;
	}

	// We set current position to the file end? so we can get the file size by calling tellg() which return the current position for reading(now the end of the file)
	std::streampos size = inputFile.tellg();

	// After getting file size set the current position to the file begining to read all data
	inputFile.seekg(0);

	// Getting the first line with a header
	std::string line;
	std::getline(inputFile, line);

	// Getting a length of this line
	int lineLength = line.length();

	// Variable for the index of the entering of the each header part(width, height or grayscale) in a full line of the header
	// P5 640 480 255 - so index for 640(width) is 3(in counts from zero), index for 480(height) is 7, for 255(grayscale) is 11
	int partBeginInd = 0;

	int width;		// width from the header
	int height;		// height from the header
	int maxPixel;	// grayscale value from the header

	int step = 0; // step number for reading width, height and grayscale - 3 steps

				  // Parsing a header line character by character
	for (int chInd = 0; chInd < lineLength; ++chInd)
	{
		// if we meet space - this is the indication of the end of some header part 
		if (line[chInd] == ' ')
		{
			switch (step)
			{
			case 0:
				// we're on 'P5 ' position
				// nothing to do
				break;
			case 1:
				//  we're on 'P5 640 ' position - getting 640 for the width 
				width = stoi(line.substr(partBeginInd, chInd - partBeginInd));
				break;
			case 2:
				//  we're on 'P5 640 480 ' position - getting 480 for the height
				height = stoi(line.substr(partBeginInd, chInd - partBeginInd));
				break;
			default:
				break;
			};

			// Setting 'partBeginInd' to the next position after the space character
			partBeginInd = chInd + 1;
			// Incrementing step, width->then height-> then grayscale
			++step;
		}
	}

	// Parsing grayscale value out of the loop because no any space symbol after 255(grayscale)
	// We're simply reading the finnal line part after the final space symbol (before 255)
	maxPixel = stoi(line.substr(partBeginInd, lineLength - partBeginInd));

	// Creating an array of bytes
	unsigned char* memblock = new unsigned char[size - inputFile.tellg()];
	inputFile.read((char*)memblock, size - inputFile.tellg());
	inputFile.close();

	int lastIndex = file.find_last_of("_");

	std::string oFile = file.substr(0, lastIndex) + "_a.pgm";
	std::ofstream outputFile(oFile.c_str());

	// Write out pgm header

	outputFile << "P2\n";
	outputFile << "# Binary to Ascii conversion result\n";
	outputFile << width << " " << height << "\n";
	outputFile << maxPixel << "\n";

	// Iterate over pixel values from remaining bytes in binary file

	for (int index = 0; index < width * height; ++index)
	{
		// Write out pixel value in standard integer ascii representation
		outputFile << (int)memblock[index];

		if (((index + 1) % width == 0) && index > 0)
		{
			outputFile << "\n";
		}
		else
		{
			outputFile << " ";
		}

	}

	delete[] memblock;

	outputFile.close();
}

void asciiToSvd(std::string file)
{
	// compilation of the filename
	int lastIndex = file.find_last_of(".pgm");
	std::string header = file.substr(0, lastIndex - 3) + "_header.txt";
	std::string svd = file.substr(0, lastIndex - 3) + "_svd.txt";
	std::ifstream inputFile(file);

	if (!inputFile)
	{
		std::cerr << "Could not open file " << file << "\n";
		return;
	}

	std::string line;
	getline(inputFile, line);	// "P2"
	getline(inputFile, line);	// comment line

								// insert the part if '#' exists or not

	float value;
	std::stringstream ss;

	int height, width, grayscale;

	ss << inputFile.rdbuf(); // rdbuf() returns the pointer to the current stream position - now position on width value

	ss >> width >> height;
	ss >> grayscale;

	//https://en.wikipedia.org/wiki/Eigen_(C%2B%2B_library)
	Eigen::MatrixXf a(height, width);

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			ss >> value;
			a(i, j) = value;
		}
	}

	inputFile.close();

	std::ofstream headerFile(header);

	headerFile << width << " " << height << " " << grayscale;

	headerFile.close();

	Eigen::MatrixXf u(width, width), s(height, width), v(height, height);
	Eigen::VectorXf singularVals;
	Eigen::JacobiSVD<Eigen::MatrixXf> svdm(a, Eigen::ComputeFullU | Eigen::ComputeFullV);

	u = svdm.matrixU();
	v = svdm.matrixV();

	//cout << u << endl;

	singularVals = svdm.singularValues();

	for (int i = 0; i < a.rows(); ++i)
	{
		for (int j = 0; j < a.cols(); ++j)
		{
			float val = 0;

			if (i == j)
				val = singularVals(i);

			s(i, j) = val;
		}
	}

	std::ofstream svdFile(svd);
	svdFile << u << "\n" << s << "\n" << v << "\n";
	svdFile.close();

}

void svdCompress(std::string header, std::string svd, int k)
{
	std::vector<unsigned char> buffer;

	std::ifstream fheader(header);

	if (!fheader)
	{
		std::cerr << "Could not open file " << header << "\n";
		return;
	}

	int width, height, grayscale;

	fheader >> width >> height >> grayscale;
	fheader.close();

	// This can probably be condensed into a method. Add width, height, and
	// max gray scale value to the buffer

	buffer.push_back(width / 256);		//we took 258 for example and dividing int by 256 ; 258 / 256 = 1 // 258(base = 10) = 00000001 00000010 (base = 2)
	buffer.push_back(width % 256);		// 258 % 256 = 2
	buffer.push_back(height / 256);
	buffer.push_back(height % 256);
	buffer.push_back(grayscale);

	// Must store the rank so we know where to fill in values for
	// recreating the original matrix

	buffer.push_back(k / 256);
	buffer.push_back(k % 256);

	// We'll want to take the resulting compressed image and name it
	// image_b.pgm.SVD - which could take some finessing of the original
	// ascii to binary te preserve method length.

	std::string temp = "image_b.pgm.svd";
	std::ofstream output(temp, std::ofstream::binary);

	// Write to values

	for (unsigned int i = 0; i < buffer.size(); ++i)
		output.write((char*)&buffer[i], sizeof(buffer[i]));


	int rank = 0;

	std::ifstream fsvd(svd);

	if (!fsvd)
	{
		std::cerr << "Could not open file " << svd << ".\n";
		return;
	}

	// Given an m by n matrix A, U is m by m
	for (int i = 0; i < height; ++i)		// height now 480
	{
		rank = 1;
		std::string line;
		std::getline(fsvd, line);
		std::stringstream ss(line);

		for (int j = 0; j < height; ++j) // height now 480
		{
			// amount of cases when we are here = 480 * 480

			// if k < 480 * 480

			float temp;
			ss >> temp; // throw away

						// keep those values up to the rank ond disregard unneeded
						// dimensions.

			if (rank <= k)
			{
				output.write(reinterpret_cast<char*>(&temp), sizeof temp);
				++rank;
			}

		}



	}

	// Given an m by n matrix A, Σ (S) is m by n

	rank = 1;
	for (int i = 0; i < height; ++i)
	{
		std::string line;
		std::getline(fsvd, line);
		std::stringstream ss(line);

		for (int j = 0; j < width; ++j)
		{
			float temp;
			ss >> temp;	// throw away

						// keep the singlar values along the diagonal, but only up to
						// the given rank approximation. the bottom of sigma can be
						// removed

			if (i == j && rank <= k)
			{
				output.write(reinterpret_cast<char*>(&temp), sizeof temp);
				++rank;
			}

		}

	}

	// Given an m by n matrix A, V is n by n
	for (int i = 0; i < width; ++i)
	{
		rank = 1;

		std::string line;
		std::getline(fsvd, line);

		std::stringstream ss(line);

		for (int j = 0; j < width; ++j)
		{
			float temp;
			ss >> temp; // thow away

						// keep those values up to the rank and disregard unneeded
						// dimensions. the right of V can be removed (bottom of V^T)

			if (rank <= k)
			{
				output.write(reinterpret_cast<char*>(&temp), sizeof temp);
				++rank;
			}

		}

	}

	fsvd.close();

	output.close();

}

void svdDecompress(std::string file)
{
	// Open binary file. Read gray scale value, width, height of image
	std::ifstream inputFile(file.c_str(), std::ifstream::binary | std::ifstream::ate);

	if (!inputFile)
	{
		std::cerr << "Could not find file " << file << "\n";
		return;
	}

	const int bytesInHeader = 7;
	unsigned char* memblock = new unsigned char[bytesInHeader];

	inputFile.seekg(0, std::ios::beg);
	inputFile.read((char*)memblock, bytesInHeader);

	// Read SVD values and write to full matrices based on height and
	// width dimensions for multiplication.

	/*

	First 5 bytes of data consist of header information

	* Bytes 0-1: Width

	* Bytes 2-3: Height

	* Byte 4: Maximum Pixel Value

	* Bytes 5-6: Rank approximation

	*/

	int width = memblock[0] * 256 + memblock[1];	// n
	int height = memblock[2] * 256 + memblock[3];	// m
	int maxPixel = memblock[4];
	int rank = memblock[5] * 256 + memblock[6];

	// Matrix U: m by m (height by height)

	Eigen::MatrixXf u(height, height);

	int k = 1;

	for (int i = 0; i < height; ++i)
	{
		k = 1;

		for (int j = 0; j < height; ++j)
		{
			float val = 0;

			if (k <= rank)
			{
				inputFile.read(reinterpret_cast<char*>(&val), sizeof val);
				++k;
			}

			u(i, j) = val;

		}

	}


	// Matrix S: m by n (height by width)

	Eigen::MatrixXf s(height, width);

	k = 1;

	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			float val = 0;

			if (k <= rank && i == j)
			{
				inputFile.read(reinterpret_cast<char*>(&val), sizeof val);
				++k;
			}

			s(i, j) = val;

		}

	}


	// Matrix V: n by n (width by width)

	Eigen::MatrixXf v(width, width);

	for (int i = 0; i < width; ++i)
	{
		k = 1;

		for (int j = 0; j < width; ++j)
		{
			float val = 0;

			if (k <= rank)
			{
				inputFile.read(reinterpret_cast<char*>(&val), sizeof val);
			}

			v(i, j) = val;
			++k;

		}

	}

	delete[] memblock;

	// Multiply matrices to get the original image in matrix A

	Eigen::MatrixXf vT = v.transpose();
	Eigen::MatrixXf a = u * s * vT;

	// Write gray scale value, width, height, matrix A into ASCII PGM file
	//! @todo I think k needs renamed to be what the k value was used
	//! to produce this image.

	std::ofstream outputFile("image_k.pgm");

	// Header

	outputFile << "P2\n";
	outputFile << "# SVD Decompressing output!\n";
	outputFile << width << " " << height << "\n";
	outputFile << maxPixel << "\n";

	// Approximate original image

	for (int i = 0; i < a.rows(); ++i)
	{
		for (int j = 0; j < a.cols(); ++j)
		{
			float temp = a(i, j);
			int out;

			if (temp >= 0)
			{
				out = (int)(temp + 0.5);
			}
			else
			{
				out = (int)(temp - 0.5);
			}
			outputFile << out << " ";

		}

		outputFile << "\n";

	}

	outputFile.close();

}

int main(int argc, char* argv[])
{
	char variant[10];
	char pgm_file_name[80];
	char header_file_name[80];
	char svd_file_name[80];
	char k_str[80];

	strcpy(variant, argv[1]);

	switch (variant[0])
	{
	case '1':
		strcpy(pgm_file_name, argv[2]);
		asciiToBinary(pgm_file_name);
		break;
	case '2':
		strcpy(pgm_file_name, argv[2]);
		binaryToAscii(pgm_file_name);
		break;
	case '3':
		strcpy(header_file_name, argv[2]);
		strcpy(svd_file_name, argv[3]);
		strcpy(k_str, argv[4]);

		svdCompress(header_file_name, svd_file_name, atoi(k_str));
		break;
	case '4':
		strcpy(pgm_file_name, argv[2]);
		svdDecompress(pgm_file_name);
		break;
	case '5':
		strcpy(pgm_file_name, argv[2]);
		asciiToSvd(pgm_file_name);
		break;
	default:
		break;
	}

	return 0;

}
