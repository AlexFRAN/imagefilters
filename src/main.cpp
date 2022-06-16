#include <iostream>
#include <vector>

//#include "include/af_image.h"
#include "include/af_image_threads.h"
#include "include/af_timer.h"


int main()
{
    af::Timer timer;

    af::Image original;
    af::Image padded;
    af::Image padded2;
    af::Image gaussian;
    af::Image sharpened;
    af::Image soebelTop;
    af::Image soebelLeft;
    timer.Stop();

    original.load("assets/nyc.jpg");
    //timer.Stop();
    //padded.create(original.getWidth(), original.getHeight(), original.getChannels());
    //original.copy(&padded);    // Plain copy, byte by byte
    //original.copyRgb(&padded);   // Rgb-copy, rgb-struct by rgb-struct (three byte groups)
    original.padImageRgb(&padded, 2);
    original.padImageRgb(&padded2, 2);
    timer.Stop();

    std::vector<std::vector<float>> gaussianKernel = {
        {1,2,3,2,1},
        {2,3,6,3,2},
        {3,6,8,6,3},
        {2,3,6,3,2},
        {1,2,3,2,1}
    };

    std::vector<std::vector<float>> sharpeningKernel = {
        { 0,-0.5F, 0},
        {-0.5F, 3,-0.5F},
        { 0,-0.5F, 0}
    };

    /*std::vector<std::vector<float>> soebelTopKernel = {
        { 2, 3, 6, 3, 2},
        { 1, 2, 3, 2, 1},
        { 0, 0, 0, 0, 0},
        {-1,-2,-3,-2,-1},
        {-2,-3,-6,-3,-2}
    };*/
    std::vector<std::vector<float>> soebelTopKernel = {
        { 1, 2, 1},
        { 0, 0, 0},
        {-1,-2,-1},
    };

    std::vector<std::vector<float>> soebelLeftKernel = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1},
    };

    gaussian.create(original.getWidth(), original.getHeight(), original.getChannels());
    padded.applyKernel(gaussianKernel, &gaussian);
    padded.write("assets/nyc_padded.jpg");
    timer.Stop();

    gaussian.write("assets/nyc_gaussian.jpg");
    timer.Stop();

    sharpened.create(original.getWidth(), original.getHeight(), original.getChannels());
    padded2.applyKernel(sharpeningKernel, &sharpened);
    sharpened.write("assets/nyc_sharpened.jpg");
    timer.Stop();

    soebelTop.create(original.getWidth(), original.getHeight(), original.getChannels());
    padded.applyKernel(soebelTopKernel, &soebelTop);
    soebelTop.write("assets/nyc_soebel_top.jpg");

    soebelLeft.create(original.getWidth(), original.getHeight(), original.getChannels());
    padded.applyKernel(soebelLeftKernel, &soebelLeft);
    soebelLeft.write("assets/nyc_soebel_left.jpg");

    return 0;
}