#pragma once

namespace LibGraphics
{
    static const char* CANNY_EDGE_DETECT_BLUR_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_;  // Texture resolution
        out vec4 FragColor;

        const float kernel[25] = float[](
            1.0, 4.0, 6.0, 4.0, 1.0,
            4.0, 16.0, 24.0, 16.0, 4.0,
            6.0, 24.0, 36.0, 24.0, 6.0,
            4.0, 16.0, 24.0, 16.0, 4.0,
            1.0, 4.0, 6.0, 4.0, 1.0
        );

        // Apply Gaussian blur using the kernel
        void main() {
            vec2 texelSize = 1.0 / _Resolution_;
            vec3 color = vec3(0.0);

            int index = 0;
            for (int i = -2; i <= 2; ++i) {
                for (int j = -2; j <= 2; ++j) {
                    color += texture(texture1, TexCoord + texelSize * vec2(i, j)).rgb * kernel[index++] / 256.0;
                }
            }

            FragColor = vec4(color, 1.0);
        }

    )";

	static const char* CANNY_EDGE_DETECT_SOBEL_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_; // Texture resolution
        out vec4 FragColor;

        const mat3 Gx = mat3(-1.0, 0.0, 1.0, 
                             -2.0, 0.0, 2.0, 
                             -1.0, 0.0, 1.0);

        const mat3 Gy = mat3(-1.0, -2.0, -1.0, 
                              0.0, 0.0, 0.0, 
                              1.0, 2.0, 1.0);

        void main() {
            vec2 texelSize = 1.0 / _Resolution_;

            // Sobel X and Y gradients
            float sobelX = 0.0;
            float sobelY = 0.0;

            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    vec3 rgb = texture(texture1, TexCoord + texelSize * vec2(i, j)).rgb;
                    sobelX += rgb.r * Gx[i+1][j+1];
                    sobelY += rgb.r * Gy[i+1][j+1];
                }
            }

            // Compute gradient magnitude and direction
            float magnitude = length(vec2(sobelX, sobelY));
            float direction = atan(sobelY, sobelX);

            // Store gradient magnitude and direction in the red and green channels
            FragColor = vec4(magnitude, direction, 0.0, 1.0);
        }

	)";

    static const char* CANNY_EDGE_DETECT_THRESHOLD_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D gradientData; // Contains magnitude and direction from the Sobel shader
        uniform float uEdgeThresholdLow;     // Low threshold for weak edges
        uniform float uEdgeThresholdHigh;    // High threshold for strong edges
        out vec4 FragColor;

        void main() {
            // Retrieve the gradient magnitude from the texture (stored in the red channel)
            float magnitude = texture(gradientData, TexCoord).r;

            // Classify the pixel as strong, weak, or not an edge
            if (magnitude >= uEdgeThresholdHigh) {
                FragColor = vec4(1.0); // Strong edge (white)
            } else if (magnitude >= uEdgeThresholdLow) {
                FragColor = vec4(0.5); // Weak edge (gray)
            } else {
                FragColor = vec4(0.0); // No edge (black)
            }
        }
    )";

    static const char* CANNY_EDGE_DETECT_HYSTERIESIS_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D thresholdedData; // Contains the classified edges (strong, weak, none)
        uniform vec2 _Resolution_;  // Texture resolution
        out vec4 FragColor;

        // Check if a pixel is a strong edge
        bool isStrongEdge(vec2 pos) {
            return texture(thresholdedData, pos).r == 1.0;
        }

        // Check if a pixel is a weak edge
        bool isWeakEdge(vec2 pos) {
            return texture(thresholdedData, pos).r == 0.5;
        }

        // Check the 8-connected neighbors for strong edges
        bool isAdjacentToStrongEdge(vec2 pos) {
            vec2 offsets[8] = vec2[](
                vec2(1.0, 0.0), vec2(-1.0, 0.0),
                vec2(0.0, 1.0), vec2(0.0, -1.0),
                vec2(1.0, 1.0), vec2(-1.0, -1.0),
                vec2(1.0, -1.0), vec2(-1.0, 1.0)
            );

            for (int i = 0; i < 8; i++) {
                if (isStrongEdge(pos + offsets[i] / _Resolution_)) {
                    return true;
                }
            }
            return false;
        }

        void main() {
            // If the pixel is a strong edge, keep it
            if (isStrongEdge(TexCoord)) {
                FragColor = vec4(1.0); // Strong edge (white)
            }
            // If the pixel is a weak edge and is adjacent to a strong edge, convert it to a strong edge
            else if (isWeakEdge(TexCoord) && isAdjacentToStrongEdge(TexCoord)) {
                FragColor = vec4(1.0); // Weak edge converted to strong edge (white)
            }
            // Otherwise, no edge
            else {
                FragColor = vec4(0.0); // No edge (black)
            }
        }
    )";
}