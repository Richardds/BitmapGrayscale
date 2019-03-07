/**
 * Color bitmap to grayscale converter
 *
 * Richard Boldiš
 *
 * https://en.wikipedia.org/wiki/BMP_file_format
 */

#include <iostream>
#include <iomanip>
#include <fstream>

// Bitmap compression type
#define BI_RGB             0 // Supported
#define BI_RLE8            1
#define BI_RLE4            2
#define BI_BITFIELDS       3
#define BI_JPEG            4
#define BI_PNG             5
#define BI_ALPHABITFIELDS  6
#define BI_CMYK           11
#define BI_CMYKRLE8       12
#define BI_CMYKRLE4       13

using namespace std;

struct __attribute__((__packed__)) RGBColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

struct __attribute__((__packed__)) BitmapInfoHeader {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t color_planes;
    uint16_t color_depth;
    uint32_t compression;
    uint32_t bitmap_size;
    uint32_t x_resolution;
    uint32_t y_resolution;
    uint32_t colors;
    uint32_t important_colors;
};

struct __attribute__((__packed__)) BitmapFileHeader {
    uint8_t magic1;
    uint8_t magic2;
    uint32_t total_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t bitmap_offset;
};

inline uint8_t grayscale(RGBColor color) {
	return 0.1 * color.red + 0.6 * color.green + 0.3 * color.blue;
}

int main(void) {
    string filename;
    BitmapFileHeader header;
    BitmapInfoHeader info;

    cout << "Image path: " << endl;
    cin >> filename;
    
    ifstream ifStream(filename, ios::in | ios::binary);
    
    if (ifStream.is_open()) {
        ifStream.read((char*)&header, sizeof(BitmapFileHeader));
        
		if (header.magic1 != 'B' || header.magic2 != 'M') {
			ifStream.close();
			cout << "Invalid file format! Magic header mismatch." << endl;
			return 1;
		}
		
		ifStream.read((char*)&info, sizeof(BitmapInfoHeader));
		
		if (info.color_depth != 24) {
			ifStream.close();
			cout << "Unsupported color depth (" << info.color_depth << ")!" << endl;
			return 1;
		}
		
		if (info.compression != BI_RGB) {
			ifStream.close();
			cout << "Unsupported compression type (" << info.compression << ")!" << endl;
			return 1;
		}
		
		if (info.bitmap_size != 3 * info.width * info.height) {
			ifStream.close();
			cout << "File is corrupted!" << endl;
			return 1;
		}
		
        cout << "File size: " << header.total_size << endl;
        cout << "Image size: " << info.width << "x" << info.height << endl;
        cout << "Surface count: " << info.color_planes << endl;
        cout << "Color depth: " << info.color_depth << endl;
        cout << "Compression: RGB" << endl;
        cout << "Bitmap size: " << info.bitmap_size << endl;
        cout << "Resolution: " << info.x_resolution << "x" << info.y_resolution << " px/m" << endl;
		cout << endl;
		
		ifStream.seekg(header.bitmap_offset, ios_base::beg);
		
		header.total_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + (3 * info.width * info.height);
		header.bitmap_offset = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
		
		info.size = sizeof(BitmapInfoHeader);
		info.color_planes = 1;
		info.compression = BI_RGB;
		info.bitmap_size = 3 * info.width * info.height;
		info.colors = 0xFFFFFF;
		info.important_colors = 0; // All colors are important
		
		ofstream ofStream(filename + ".grayscale.bmp", ios::out | ios::binary);
		ofStream.write((char*)&header, sizeof(BitmapFileHeader));
		ofStream.write((char*)&info, sizeof(BitmapInfoHeader));
		
		RGBColor rgbIn, rgbOut;
		uint32_t currentPixel = 0;
		uint32_t totalPixels = info.width * info.height;
		uint32_t statusRenderLimit = max(info.width, info.height);
		
		for (uint32_t i = 0; i < info.height; i++) {
			for (uint32_t j = 0; j < info.width; j++) {
				ifStream.read((char*)&rgbIn, sizeof(RGBColor));
				rgbOut.red = rgbOut.green = rgbOut.blue = grayscale(rgbIn);
				ofStream.write((char*)&rgbOut, sizeof(RGBColor));
				currentPixel++;
				
				// Status rendering
				if ((currentPixel % statusRenderLimit) == 0) { // Do not let status rendering eat all processing power
					cout << "\r";
					cout << "Progress: " << fixed << setprecision(3) << (((float)currentPixel / (float)totalPixels) * 100) << "%";
				}
			}
		}
		
		cout << endl;
		
        ifStream.close();
		ofStream.close();
    }
    
    return 0;
}

