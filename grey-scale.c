#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

void convert_to_grayscale(JSAMPARRAY buffer, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            JSAMPROW row = buffer[y];
            JSAMPLE *pixel = &row[x * 3];
            // Calculate the grayscale value
            int gray = (int)(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
            // Set all color components to the grayscale value
            pixel[0] = pixel[1] = pixel[2] = gray;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    // Variables for the decompression process
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    // Open the input file
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        perror("Error opening input file");
        return 1;
    }

    // Set up error handling
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, input_file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    int height = cinfo.output_height;
    int pixel_size = cinfo.output_components;

    // Allocate memory for the image buffer
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, width * pixel_size, height);

    // Read the image data
    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, height - cinfo.output_scanline);
    }

    // Convert the image to grayscale
    convert_to_grayscale(buffer, width, height);

    // Variables for the compression process
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr_out;

    // Open the output file
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Error opening output file");
        return 1;
    }

    // Set up error handling
    cinfo_out.err = jpeg_std_error(&jerr_out);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, output_file);

    // Set the parameters for compression
    cinfo_out.image_width = width;
    cinfo_out.image_height = height;
    cinfo_out.input_components = pixel_size;
    cinfo_out.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);

    // Write the image data
    while (cinfo_out.next_scanline < cinfo_out.image_height) {
        jpeg_write_scanlines(&cinfo_out, buffer + cinfo_out.next_scanline, height - cinfo_out.next_scanline);
    }

    // Finish compression and clean up
    jpeg_finish_compress(&cinfo_out);
    jpeg_destroy_compress(&cinfo_out);
    fclose(output_file);

    // Clean up decompression
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);

    return 0;
}

