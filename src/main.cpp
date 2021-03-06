#include <iostream>
#include <memory>

#include <math.h>
#include <string.h>

#include <args.hxx>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

int main(int argc, const char *const *argv) {
    args::ArgumentParser parser("Create sdf image.");
    args::Flag argDistanceColor(parser, "dist-color", "Output distance into rgb.", {"dist-color"});
    args::ValueFlag<unsigned short> argRadius(parser, "radius", "Sampling radius.", {'r', "radius"}, 8);
    args::ValueFlag<unsigned short> argWidth(parser, "width", "Output image width.", {'w', "width"});
    args::ValueFlag<unsigned short> argHeight(parser, "height", "Output image height.", {'h', "heigt"});
    args::Positional<std::string> argInput(parser, "input", "Input image.",
                                           args::Options::Required);
    args::Positional<std::string> argOutput(parser, "output", "Output image.",
                                            args::Options::Required);
    args::CompletionFlag completion(parser, {"complete"});
    try {
        parser.ParseCLI(argc, argv);
    } catch (const args::Completion &e) {
        std::cout << e.what();
        return 0;
    } catch (const args::RequiredError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 0;
    } catch (const args::ParseError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    int width = 0, height = 0, comp = 4;
    uint8_t *srcImage =
        stbi_load(argInput.Get().c_str(), &width, &height, &comp, comp);
    auto size = width * height * comp;
    auto dstImage = std::make_unique<uint8_t[]>(size);

    auto r = args::get(argRadius);
    auto s = 0.125f / (r * r);
    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            auto index = comp * (y * width + x);

            auto a0 = srcImage[index + 3];

            if(argDistanceColor) {
                dstImage[index + 3] = 0xff;
            } else {
                dstImage[index + 0] = srcImage[index + 0];
                dstImage[index + 1] = srcImage[index + 1];
                dstImage[index + 2] = srcImage[index + 2];
            }

            auto minDistSq = INT32_MAX;
            for (auto yy = -r; yy <= r; yy++) {
                for (auto xx = -r; xx <= r; xx++) {
                    auto xxx = xx + x;
                    if (xxx < 0 || xxx >= width) {
                        continue;
                    }

                    auto yyy = yy + y;
                    if (yyy < 0 || yyy >= height) {
                        continue;
                    }

                    auto a = srcImage[comp * (yyy * width + xxx) + 3];
                    auto diff = abs(static_cast<int32_t>(a) - a0);
                    if (diff > 128) {
                        auto distSq = xx * xx + yy * yy;
                        if (minDistSq > distSq) {
                            minDistSq = distSq;
                        }
                    }
                }
            }

            int result = 0;
            if (minDistSq == INT32_MAX) {
                result = a0 < 128 ? 0x00 : 0xff;
            } else {
                auto distSq = sqrt(s * minDistSq);
                distSq = ((a0 < 128) ? -distSq : distSq) + 0.5f;
                result = static_cast<uint8_t>(round(255.0f * distSq));
            }

            if(argDistanceColor) {
                dstImage[index + 0] = result;
                dstImage[index + 1] = result;
                dstImage[index + 2] = result;
            } else {
                dstImage[index + 3] = result;
            }
        }
    }

    stbi_image_free(srcImage);

    if (argWidth || argHeight) {
        auto w = argWidth ? args::get(argWidth) : width;
        auto h = argHeight ? args::get(argHeight) : height;
        auto img = std::make_unique<uint8_t[]>(w * h * comp);
        stbir_resize_uint8(dstImage.get(), width, height, 0, img.get(), w, h, 0, comp);
        stbi_write_png(argOutput.Get().c_str(), w, h, comp, img.get(), 0);
    } else {
        stbi_write_png(argOutput.Get().c_str(), width, height, comp, dstImage.get(), 0);
    }

    return 0;
}
