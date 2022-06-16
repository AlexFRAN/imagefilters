#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

#include <vector>
#include <thread>


namespace af
{
    struct rgb
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct rgb_int
    {
        int r;
        int g;
        int b;
    };

    struct rgb_float
    {
        float r;
        float g;
        float b;
    };

    struct rgba
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };


    class Image
    {
    private:
        unsigned char* m_image;
        int m_width;
        int m_height;
        int m_channels;
        int m_padding = 0;  // If the image is padded / padding has been applied and saved in the current image-object, this defines the padding per side (at the moment only padding wich has the same size for each side is supported)
        int m_thread_count;
        std::vector<std::vector<float>> m_kernel;
        Image* m_kernel_image;

    public:
        Image()
        {
            m_image = nullptr;
            m_thread_count = std::thread::hardware_concurrency();
        }

        ~Image()
        {
            if(m_image != nullptr)
            {
                free(m_image);
                m_image = nullptr;
            }
        }

        // Load image from file
        void load(const char* path)
        {
            m_image = stbi_load(path, &m_width, &m_height, &m_channels, 0);
        }

        // Create image from scratch
        void create(int width, int height, int channels)
        {
            m_width = width;
            m_height = height;
            m_channels = channels;
            m_image = (unsigned char*)malloc(width * height * channels);
        }

        // Get the size (in bytes)
        int getSize()
        {
            return (m_width * m_height * m_channels);
        }

        // Get the width
        int getWidth()
        {
            return m_width;
        }

        // Get the height
        int getHeight()
        {
            return m_height;
        }

        // Get the number of channels
        int getChannels()
        {
            return m_channels;
        }

        // Get the padding per side
        int getPadding()
        {
            return m_padding;
        }

        // Get the pointer to the image
        unsigned char* getImage()
        {
            return m_image;
        }

        // Get the values of a single pixel, rgb version
        rgb getPixelRgb(int row, int col)
        {
            return {
                *(m_image + (row * col * m_channels)),
                *(m_image + (row * col * m_channels) + 1),
                *(m_image + (row * col * m_channels) + 2)
            };
        }

        // Get the values of a single pixel, rgba version
        rgba getPixelRgba(int row, int col)
        {
            return {
                *(m_image + (row * col * m_channels)),
                *(m_image + (row * col * m_channels) + 1),
                *(m_image + (row * col * m_channels) + 2),
                *(m_image + (row * col * m_channels) + 3)
            };
        }

        // Set the pixel on an rgb image
        void setPixelRgb(int row, int col, rgb value)
        {
            *(m_image + (row * m_width * m_channels) + (col * m_channels)) = value.r;
            *(m_image + (row * m_width * m_channels) + (col * m_channels) + 1) = value.g;
            *(m_image + (row * m_width * m_channels) + (col * m_channels) + 2) = value.b;
        }

        // Set the pixel on an rgba image
        void setPixelRgba(int row, int col, rgba value)
        {
            *(m_image + (row * m_width * m_channels) + (col * m_channels)) = value.r;
            *(m_image + (row * m_width * m_channels) + (col * m_channels) + 1) = value.g;
            *(m_image + (row * m_width * m_channels) + (col * m_channels) + 2) = value.b;
            *(m_image + (row * m_width * m_channels) + (col * m_channels) + 3) = value.a;
        }

        // Set a 8-bit value
        void setRaw(int position, unsigned char value)
        {
            *(m_image + position) = value;
        }

        // Copy the current image to another image object
        void copy(Image* image)
        {
            // First check dimensions. TODO: Error-handling
            if((*image).getWidth() != m_width ||
               (*image).getHeight() != m_height ||
               (*image).getChannels() != m_channels)
            {
                return;
            }

            for(int i = 0; i < (m_width * m_height * m_channels); i++)
            {
                (*image).setRaw(i, *(m_image + i));
            }
        }

        // Copy the current iamge to another image object, rgb version
        void copyRgb(Image* image)
        {
            // First check dimensions. TODO: Error-handling
            if((*image).getWidth() != m_width ||
               (*image).getHeight() != m_height ||
               (*image).getChannels() != m_channels ||
               m_channels != 3)
            {
                return;
            }

            for(int row = 0; row < m_height; row++)
            {
                for(int col = 0; col < m_width; col++)
                {
                    (*image).setPixelRgb(row, col, {
                        *(m_image + (row * m_width * m_channels) + (col * m_channels)),
                        *(m_image + (row * m_width * m_channels) + (col * m_channels) + 1),
                        *(m_image + (row * m_width * m_channels) + (col * m_channels) + 2)
                    });
                }
            }
        }

        // Pad the image and copy to new image object, rgb version, padding is inteded as padding per side
        void padImageRgb(Image* image, int padding)
        {
            // TODO: Error handling, process does not work if no image is loaded or the padding is larger than the image itself
            if(!m_width || !m_image || padding > m_width || padding > m_height)
            {
                return;
            }

            int new_width = m_width + padding * 2;
            int new_height = m_height + padding * 2;

            image->destroy();    // First destroy everything inside the new image object
            image->create(new_width, new_height, m_channels);  // Then allocate the new image using the padded image-size
            unsigned char* new_image = image->getImage();   // Write directly to the memory, bypassing all method calls
            
            for(int row = 0; row < new_height; row++)
            {
                int original_row = row - padding;

                // TODO: Branchless version!
                if(row < padding)
                {
                    original_row = padding - row - 1;
                }
                else if(row >= (m_height + padding))
                {
                    original_row = m_height - (row - padding - m_height) - 1;
                }

                for(int col = 0; col < new_width; col++)
                {
                    int original_col = col - padding;

                    // TODO: Branchless version!
                    if(col < padding)
                    {
                        original_col = padding - col - 1;
                    }
                    else if(col >= (m_width + padding))
                    {
                        original_col = m_width - (col - padding - m_width) - 1;
                    }

                    // Copy old pixel to new image
                    *(new_image + (row * new_width + col) * m_channels) = *(m_image + (original_row * m_width + original_col) * m_channels);
                    *(new_image + (row * new_width + col) * m_channels + 1) = *(m_image + (original_row * m_width + original_col) * m_channels + 1);
                    *(new_image + (row * new_width + col) * m_channels + 2) = *(m_image + (original_row * m_width + original_col) * m_channels + 2);
                }
            }

            image->setPadding(padding);
        }

        // Just set the padding property
        void setPadding(int padding)
        {
            m_padding = padding;
        }

        // Get the total sum of a kernel
        float getKernelSum(std::vector<std::vector<float>> &kernel)
        {
            float sum = 0;

            for(int row = 0; row < kernel.size(); row++)
            {
                for(int col = 0; col < kernel.at(row).size(); col++)
                {
                    sum += kernel.at(row).at(col);
                }
            }

            return sum;
        }

        void kernelThread(int start_row, int end_row)
        {
            float kernel_sum = getKernelSum(m_kernel);
            kernel_sum = kernel_sum == 0 ? 1 : kernel_sum;
            rgb_float current_pixel_sum = {0.0F,0.0F,0.0F};
            int row, col, kernel_row, kernel_col;   // Declare counter variables to prevent reallocation of (stack) memory over and over again
            int row_offset, col_offset;     // How many rows / cols is the current pixel distant from the center one?
            int center = (m_kernel.size() - 1) / 2;
            int new_width = m_kernel_image->getWidth();
            unsigned char* new_image = m_kernel_image->getImage();
            
            for(row = start_row; row < end_row; row++)
            {
                for(col = m_padding; col < (m_width - m_padding); col++)
                {
                    current_pixel_sum = {0.0F,0.0F,0.0F};

                    for(kernel_row = 0; kernel_row < m_kernel.size(); kernel_row++)
                    {
                        row_offset = kernel_row - center;

                        for(kernel_col = 0; kernel_col < m_kernel.at(kernel_row).size(); kernel_col++)
                        {
                            col_offset = kernel_col - center;
                            current_pixel_sum.r += *(m_image + (row + row_offset) * m_width * m_channels + (col + col_offset) * m_channels) * m_kernel.at(kernel_row).at(kernel_col);
                            current_pixel_sum.g += *(m_image + (row + row_offset) * m_width * m_channels + (col + col_offset) * m_channels + 1) * m_kernel.at(kernel_row).at(kernel_col);
                            current_pixel_sum.b += *(m_image + (row + row_offset) * m_width * m_channels + (col + col_offset) * m_channels + 2) * m_kernel.at(kernel_row).at(kernel_col);
                        }
                    }
                    
                    current_pixel_sum.r = (int)(current_pixel_sum.r / kernel_sum);
                    current_pixel_sum.g = (int)(current_pixel_sum.g / kernel_sum);
                    current_pixel_sum.b = (int)(current_pixel_sum.b / kernel_sum);

                    if(current_pixel_sum.r < 0) {current_pixel_sum.r = 0;}
                    if(current_pixel_sum.r > 255) {current_pixel_sum.r = 255;}
                    if(current_pixel_sum.g < 0) {current_pixel_sum.g = 0;}
                    if(current_pixel_sum.g > 255) {current_pixel_sum.g = 255;}
                    if(current_pixel_sum.b < 0) {current_pixel_sum.b = 0;}
                    if(current_pixel_sum.b > 255) {current_pixel_sum.b = 255;}
                    
                    *(new_image + (row - m_padding) * new_width * m_channels + (col - m_padding) * m_channels) = (uint8_t)current_pixel_sum.r;
                    *(new_image + (row - m_padding) * new_width * m_channels + (col - m_padding) * m_channels + 1) = (uint8_t)current_pixel_sum.g;
                    *(new_image + (row - m_padding) * new_width * m_channels + (col - m_padding) * m_channels + 2) = (uint8_t)current_pixel_sum.b;
                }
            }
        }

        // Apply a kernel to the current (padded) image and save into a new image object
        void applyKernel(std::vector<std::vector<float>> &kernel, Image* image)
        {
            if(kernel.size() % 2 == 0 ||
               (kernel.size() - 1) / 2 > m_padding ||
               kernel.at(0).size() % 2 == 0 ||
               (kernel.at(0).size() - 1) / 2 > m_padding ||
               m_width != (image->getWidth() + m_padding * 2) ||
               m_height != (image->getHeight() + m_padding * 2))
            {
                return; // TODO: Error-handling
            }

            m_kernel = kernel;
            m_kernel_image = image;
            int rows_per_thread = (int)(m_height - m_padding * 2) / m_thread_count;
            int rows_last_thread = rows_per_thread + (m_height - m_padding * 2) % m_thread_count;
            std::vector<std::thread> threads;

            for(int i = 0; i < m_thread_count; i++)
            {
                int rows_current_thread = i < (m_thread_count - 1) ? rows_per_thread : rows_last_thread;
                int start_row = rows_per_thread * i + m_padding;
                int end_row = start_row + rows_current_thread;

                threads.push_back(std::thread(&Image::kernelThread, this, start_row, end_row));
            }

            for(int i = 0; i < m_thread_count; i++)
            {
                threads.at(i).join();
            }
        }


        // Write image to file TODO: Support multiple image formats
        void write(const char* path)
        {
            stbi_write_jpg(path, m_width, m_height, m_channels, m_image, 100);
        }

        // Delete the current image
        void destroy()
        {
            if(m_image != nullptr)
            {
                free(m_image);
                m_image = nullptr;
                m_width = 0;
                m_height = 0;
                m_channels = 0;
            }
        }
    };
};