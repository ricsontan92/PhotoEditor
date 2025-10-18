#pragma once

#include "CannyShaders.h"

namespace LibGraphics
{
    static const char* GAUSSIAN_BLUR_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uBlurScale=1.0; // 1 to infinity
        out vec4 FragColor;

        void main() {
            vec2 offsets[9] = vec2[](
                vec2(-1,  1), vec2(0,  1), vec2(1,  1),
                vec2(-1,  0), vec2(0,  0), vec2(1,  0),
                vec2(-1, -1), vec2(0, -1), vec2(1, -1)
            );

            float kernel[9] = float[](
                1.0 / 16, 2.0 / 16, 1.0 / 16,
                2.0 / 16, 4.0 / 16, 2.0 / 16,
                1.0 / 16, 2.0 / 16, 1.0 / 16
            );

            vec3 color = vec3(0.0);
            for (int i = 0; i < 9; i++) {
                color += texture(texture1, TexCoord + offsets[i] * uBlurScale / textureSize(texture1, 0)).rgb * kernel[i];
            }

            FragColor = vec4(color, 1.0);
        }
    )";

    static const char* RADIAL_BLUR_SHADER = R"(
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform sampler2D uTexture; // Input texture
        uniform vec2 uBlurCenter = vec2(0.5, 0.5); // Center of the blur in normalized coordinates (0.0 to 1.0)
        uniform float uBlurStrength = 0.15; // Strength of the blur 0 ~ 1
        uniform int uBlurSteps = 10; // Number of blur samples 5 - 50

        void main()
        {
            vec2 texSize = textureSize(uTexture, 0); // Texture dimensions
            vec2 texCoord = TexCoord;
            vec2 center = uBlurCenter * texSize; // Convert to pixel space

            vec4 color = vec4(0.0);
            float weight = 0.0;

            // Accumulate samples along the radial direction
            for (int i = 0; i < uBlurSteps; i++)
            {
                float t = float(i) / float(uBlurSteps - 1);
                vec2 offset = texCoord + (texCoord - center / texSize) * t * uBlurStrength;
                color += texture(uTexture, offset);
                weight += 1.0;
            }

            // Average the accumulated color
            FragColor = color / weight;
        }
    )";

    static const char* OUTLINE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uThreshold = 0.8; // Edge detection threshold
        out vec4 FragColor;

        void main() {
            // Sample surrounding pixels to find edges
            vec3 color = texture(texture1, TexCoord).rgb;
            vec3 left = texture(texture1, TexCoord + vec2(-1.0 / 1024.0, 0.0)).rgb;
            vec3 right = texture(texture1, TexCoord + vec2(1.0 / 1024.0, 0.0)).rgb;
            vec3 up = texture(texture1, TexCoord + vec2(0.0, 1.0 / 1024.0)).rgb;
            vec3 down = texture(texture1, TexCoord + vec2(0.0, -1.0 / 1024.0)).rgb;
    
            float edge = length(color - left) + length(color - right) + length(color - up) + length(color - down);
    
            // Apply outline color based on edge strength
            if (edge > uThreshold) {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black outline
            } else {
                FragColor = vec4(color, 1.0); // Normal texture color
            }
        }
    )";

    static const char* CHROMATIC_ABERRATION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uIntensity = 0.25; // The intensity of the aberration
        out vec4 FragColor;

        void main() {
            // Apply slight offset to each color channel
            vec2 redCoord = TexCoord + vec2(uIntensity, 0.0);
            vec2 greenCoord = TexCoord;
            vec2 blueCoord = TexCoord - vec2(uIntensity, 0.0);
    
            vec3 color = vec3(
                texture(texture1, redCoord).r,
                texture(texture1, greenCoord).g,
                texture(texture1, blueCoord).b
            );
    
            FragColor = vec4(color, 1.0);
        }
    )";

    static const char* HSL_ADJUSTMENT_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uHue = 120; // Hue shift in degrees (0-360)
        uniform float uSaturation = 0.85; // Saturation factor (0-1)
        uniform float uLightness = 0.85; // Lightness factor (0-1)
        out vec4 FragColor;

        // Function to convert RGB to HSL
        vec3 rgb2hsl(vec3 c) {
            float max = max(c.r, max(c.g, c.b));
            float min = min(c.r, min(c.g, c.b));
            float h, s, l = (max + min) / 2.0;
    
            if (max == min) {
                h = 0.0;
                s = 0.0;
            } else {
                float d = max - min;
                s = l > 0.5 ? d / (2.0 - max - min) : d / (max + min);
                if (max == c.r) {
                    h = (c.g - c.b) / d + (c.g < c.b ? 6.0 : 0.0);
                } else if (max == c.g) {
                    h = (c.b - c.r) / d + 2.0;
                } else {
                    h = (c.r - c.g) / d + 4.0;
                }
                h /= 6.0;
            }
            return vec3(h, s, l);
        }

        float hue2rgb(float p, float q, float t) {
            if (t < 0.0) t += 1.0;
            if (t > 1.0) t -= 1.0;
            if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
            if (t < 1.0 / 2.0) return q;
            if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
            return p;
        }

        // Function to convert HSL to RGB
        vec3 hsl2rgb(vec3 c) {
            float h = c.r, s = c.g, l = c.b;
            float r, g, b;
    
            if (s == 0.0) {
                r = g = b = l; // Achromatic
            } else {
                float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
                float p = 2.0 * l - q;
                r = hue2rgb(p, q, h + 1.0 / 3.0);
                g = hue2rgb(p, q, h);
                b = hue2rgb(p, q, h - 1.0 / 3.0);
            }
            return vec3(r, g, b);
        }

        void main() {
            vec4 color = texture(texture1, TexCoord);
            vec3 hsl = rgb2hsl(color.rgb);
    
            // Apply hue, saturation, and lightness adjustments
            hsl.r += uHue / 360.0;
            hsl.g *= uSaturation;
            hsl.b += uLightness - 0.5;
    
            FragColor = vec4(hsl2rgb(hsl), color.a);
        }
    )";

    static const char* GRAY_SCALE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        out vec4 FragColor;

        void main() 
        {
            vec3 color = texture(texture1, TexCoord).rgb;
            float grayscale = dot(color, vec3(0.299, 0.587, 0.114));
            FragColor = vec4(vec3(grayscale), 1.0);
        }
    )";

    static const char* SEPIA_TONE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        out vec4 FragColor;

        void main() {
            vec3 color = texture(texture1, TexCoord).rgb;
            vec3 sepia = vec3(
                dot(color, vec3(0.393, 0.769, 0.189)),
                dot(color, vec3(0.349, 0.686, 0.168)),
                dot(color, vec3(0.272, 0.534, 0.131))
            );
            FragColor = vec4(sepia, 1.0);
        }
    )";

    static const char* SOBEL_EDGE_DETECT_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uThreshold; // Threshold to control sensitivity of edge detection
        out vec4 FragColor;

        void main() {
            // Fetch the neighboring pixel values
            vec4 center = texture(texture1, TexCoord);
            vec4 left = texture(texture1, TexCoord + vec2(-0.01, 0.0));
            vec4 right = texture(texture1, TexCoord + vec2(0.01, 0.0));
            vec4 top = texture(texture1, TexCoord + vec2(0.0, 0.01));
            vec4 bottom = texture(texture1, TexCoord + vec2(0.0, -0.01));
            vec4 topLeft = texture(texture1, TexCoord + vec2(-0.01, 0.01));
            vec4 topRight = texture(texture1, TexCoord + vec2(0.01, 0.01));
            vec4 bottomLeft = texture(texture1, TexCoord + vec2(-0.01, -0.01));
            vec4 bottomRight = texture(texture1, TexCoord + vec2(0.01, -0.01));

            // Sobel filter for edge detection
            float gx = (-1.0 * topLeft.r) + (1.0 * topRight.r) +
                       (-2.0 * left.r)    + (2.0 * right.r) +
                       (-1.0 * bottomLeft.r) + (1.0 * bottomRight.r);

            float gy = (-1.0 * bottomLeft.r) + (-2.0 * bottom.r) + (-1.0 * bottomRight.r) +
                       ( 1.0 * topLeft.r)    + ( 2.0 * top.r)    + ( 1.0 * topRight.r);

            // Compute gradient magnitude
            float edgeIntensity = sqrt(gx * gx + gy * gy);

            // Apply edge threshold
            float edge = edgeIntensity > uThreshold ? 1.0 : 0.0;

            // Output as a grayscale edge map
            FragColor = vec4(vec3(edge), 1.0);
        }
    )";

    static const char* PREWITT_EDGE_DETECT_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uThreshold; // Edge sensitivity
        out vec4 FragColor;

        void main() {
            vec2 texelSize = 1.0 / textureSize(texture1, 0);

            // Neighbor sampling
            vec4 topLeft = texture(texture1, TexCoord + vec2(-texelSize.x, texelSize.y));
            vec4 top = texture(texture1, TexCoord + vec2(0.0, texelSize.y));
            vec4 topRight = texture(texture1, TexCoord + vec2(texelSize.x, texelSize.y));
            vec4 left = texture(texture1, TexCoord + vec2(-texelSize.x, 0.0));
            vec4 right = texture(texture1, TexCoord + vec2(texelSize.x, 0.0));
            vec4 bottomLeft = texture(texture1, TexCoord + vec2(-texelSize.x, -texelSize.y));
            vec4 bottom = texture(texture1, TexCoord + vec2(0.0, -texelSize.y));
            vec4 bottomRight = texture(texture1, TexCoord + vec2(texelSize.x, -texelSize.y));

            // Prewitt kernels
            float gx = -1.0 * topLeft.r + 1.0 * topRight.r
                       -1.0 * left.r + 1.0 * right.r
                       -1.0 * bottomLeft.r + 1.0 * bottomRight.r;
            float gy = -1.0 * bottomLeft.r + -1.0 * bottom.r + -1.0 * bottomRight.r
                       + 1.0 * topLeft.r + 1.0 * top.r + 1.0 * topRight.r;

            // Gradient magnitude
            float edgeStrength = sqrt(gx * gx + gy * gy);

            // Apply edge threshold
            float edge = edgeStrength > uThreshold ? 1.0 : 0.0;
            FragColor = vec4(vec3(edge), 1.0);
        }
    )";

    static const char* ROBERTS_CROSS_EDGE_DETECT_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uThreshold; // Sensitivity control
        out vec4 FragColor;

        void main() {
            vec2 texelSize = 1.0 / textureSize(texture1, 0);

            // Neighbor sampling
            vec4 center = texture(texture1, TexCoord);
            vec4 bottomRight = texture(texture1, TexCoord + vec2(texelSize.x, -texelSize.y));
            vec4 right = texture(texture1, TexCoord + vec2(texelSize.x, 0.0));
            vec4 bottom = texture(texture1, TexCoord + vec2(0.0, -texelSize.y));

            // Roberts Cross kernels
            float gx = center.r - bottomRight.r;
            float gy = right.r - bottom.r;

            // Gradient magnitude
            float edgeStrength = sqrt(gx * gx + gy * gy);

            // Apply edge threshold
            float edge = edgeStrength > uThreshold ? 1.0 : 0.0;
            FragColor = vec4(vec3(edge), 1.0);
        }
    )";

    static const char* DISTORTION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uTime; // Varying time value for animation
        out vec4 FragColor;

        void main() {
            vec2 distortedUV = TexCoord + vec2(sin(TexCoord.y * 10.0 + uTime) * 0.1, 0.0);
            FragColor = texture(texture1, distortedUV);
        }
    )";

    static const char* POSTERISATION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform int uLevels = 5; // Number of color levels (e.g., 5)
        out vec4 FragColor;

        void main() {
            vec3 color = texture(texture1, TexCoord).rgb;
            color = floor(color * float(uLevels)) / float(uLevels);
            FragColor = vec4(color, 1.0);
        }
    )";

    static const char* NOISE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uNoiseAmount = 0.5;  // The intensity of the noise 0 ~ 1
        out vec4 FragColor;

        float rand(vec2 co) {
            return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
        }

        void main() {
            vec3 color = texture(texture1, TexCoord).rgb;
            float noise = rand(TexCoord) * uNoiseAmount;
            color += vec3(noise);
            FragColor = vec4(color, 1.0);
        }
    )";


    static const char* BLOOM_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uThreshold; // Brightness threshold for bloom
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
    
            // Extract bright areas by applying a threshold
            vec3 bright = color.rgb - vec3(uThreshold);
            bright = max(bright, 0.0); // Clamp to positive values
    
            // Combine the original color with the bloom
            FragColor = vec4(bright + color.rgb, color.a);
        }
    )";

    static const char* HALF_TONE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_ = vec2(1, 1); // Resolution of the screen
        uniform float uDotSize = 5;   // Size of the halftone dots
        out vec4 FragColor;

        void main() {
            vec2 coord = TexCoord * _Resolution_ / uDotSize; // Scale coordinates
            vec2 grid = floor(coord);                     // Get grid position
            vec2 fractPart = fract(coord);               // Get fractional part
            vec4 color = texture(texture1, TexCoord);

            // Calculate intensity and map to dot size
            float intensity = length(color.rgb);
            float radius = intensity * 0.5;

            // Draw the circular dots
            if (length(fractPart - 0.5) < radius) {
                FragColor = vec4(color.rgb, 1.0);
            } else {
                FragColor = vec4(1.0); // Background color
            }
        }
    )";

    static const char* VHS_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uTime = 0.5; // For animation
        out vec4 FragColor;

        void main() {
            vec2 glitchTexCoord = TexCoord;

            // Create horizontal glitch lines
            float glitchOffset = sin(glitchTexCoord.y * 50.0 + uTime * 5.0) * 0.005;
            glitchTexCoord.x += glitchOffset;

            // Apply chromatic aberration
            vec2 redShift = vec2(+0.005, 0.0);
            vec2 blueShift = vec2(-0.005, 0.0);

            vec4 red = texture(texture1, glitchTexCoord + redShift);
            vec4 green = texture(texture1, glitchTexCoord);
            vec4 blue = texture(texture1, glitchTexCoord + blueShift);

            // Combine the RGB channels with slight offsets
            FragColor = vec4(red.r, green.g, blue.b, 1.0);

            // Add glitch noise
            float noise = fract(sin(dot(TexCoord, vec2(12.9898, 78.233))) * 43758.5453);
            if (noise > 0.95) {
                FragColor.rgb += vec3(0.2); // Bright noise
            }
        }
    )";

    static const char* MOSIAC_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_; // Resolution of the screen
        uniform float uMosaicSize = 5; // Size of each mosaic cell
        out vec4 FragColor;

        void main() {
            // Scale down to mosaic grid and then back up
            vec2 mosaicCoord = floor(TexCoord * _Resolution_ / uMosaicSize) * uMosaicSize / _Resolution_;

            // Sample the texture at the center of the mosaic cell
            vec4 color = texture(texture1, mosaicCoord);

            FragColor = color;
        }
    )";

    static const char* GODS_RAY_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Main texture (scene)
        uniform vec2 uLightPosition = vec2(0.5, 0.5); // Light source position
        uniform float uIntensity = 1.0;    // Light shaft intensity
        uniform float uSize = 5.0;    // Light shaft intensity
        out vec4 FragColor;

        void main() {
            vec2 offset = TexCoord - uLightPosition; // Offset based on light position
            float distance = length(offset);
    
            // Simple attenuation model based on distance from light
            float attenuation = max(0.0, 1.0 - distance * (10 - uSize));

            // Sample the texture and modulate by attenuation
            vec4 color = texture(texture1, TexCoord);
            FragColor = color * attenuation * uIntensity;
        }
    )";

    static const char* SWIRLING_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 uCenter = vec2(0.5, 0.5);       // Center of the twirl effect
        uniform float uStrength = 1;    // Strength of the swirl effect (how much distortion)
        uniform float uTime = 0.5;        // Time for animated swirling (optional)
        out vec4 FragColor;

        void main() {
            // Calculate the offset from the uCenter
            vec2 offset = TexCoord - uCenter;

            // Calculate the angle of the offset
            float angle = atan(offset.y, offset.x);
    
            // Add a time-dependent swirl effect (creates animation)
            angle += uStrength * sin(angle + uTime * 3.0); // Adjust intensity with 'strength'

            // Calculate the new position after rotating (twirling)
            float radius = length(offset);
            vec2 newCoord = uCenter + vec2(cos(angle), sin(angle)) * radius;

            // Sample the texture at the new twirled coordinates
            vec4 color = texture(texture1, newCoord);
    
            FragColor = color;
        }
    )";

    static const char* GRADIENT_OVERLAY_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec4 uColor1 = vec4(1.0, 0.0, 0.0, 1.0); // Start color of the gradient (default: vec4(1.0, 0.0, 0.0, 1.0))
        uniform vec4 uColor2 = vec4(1.0, 0.0, 1.0, 1.0); // End color of the gradient (default: vec4(0.0, 0.0, 1.0, 1.0))
        out vec4 FragColor;

        void main() {
            float factor = TexCoord.y;
            vec4 gradientColor = mix(uColor1, uColor2, factor);
            vec4 baseColor = texture(texture1, TexCoord);
            FragColor = mix(baseColor, gradientColor, 0.5); // Blend factor default: 0.5
        }
    )";

    static const char* DYNAMIC_RIPPLE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 uCenter = vec2(0.5);    // Center of the ripple (default: vec2(0.5, 0.5))
        uniform float uTime = 0.5;           // Time for animation (default: 0.0)
        uniform float uAmplitude = 0.05;     // Amplitude of the ripple (default: 0.05)
        uniform float uFrequency = 10.0;     // Frequency of the ripple (default: 10.0)
        out vec4 FragColor;

        void main() {
            vec2 offset = TexCoord - uCenter;
            float dist = length(offset);
            float ripple = sin(dist * uFrequency - uTime) * uAmplitude;
            vec2 distortedCoord = TexCoord + normalize(offset) * ripple;
            vec4 color = texture(texture1, distortedCoord);
            FragColor = color;
        }
    )";

    static const char* EMBOSS_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_;
        uniform vec2 uTexelSize = vec2(1, 1); // Size of a texel (default: 1.0 / resolution)
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            vec4 colorX = texture(texture1, TexCoord + vec2(uTexelSize.x / _Resolution_.x, 0.0));
            vec4 colorY = texture(texture1, TexCoord + vec2(0.0, uTexelSize.y / _Resolution_.y));
            vec4 emboss = vec4((colorX.rgb - colorY.rgb) + 0.5, 1.0);
            FragColor = emboss;
        }
    )";

    static const char* GLITCH_LINES_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uTime = 0;       // Current time (default: 0.0)
        uniform float uGlitchSize = 0.1; // Size of glitch (default: 0.1)
        out vec4 FragColor;

        void main() {
            vec2 newCoord = TexCoord;
            if (fract(gl_FragCoord.y * 0.1 + uTime) < uGlitchSize) {
                newCoord.x += sin(uTime + gl_FragCoord.y * 0.1) * 0.02;
            }
            FragColor = texture(texture1, newCoord);
        }
    )";

    static const char* LENS_DISTORTION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uDistortion = 0.5; // Strength of distortion (default: 0.5)
        out vec4 FragColor;

        void main() {
            vec2 centered = TexCoord * 2.0 - 1.0;
            float radius = length(centered);
            vec2 distorted = centered * (1.0 + uDistortion * radius * radius);
            FragColor = texture(texture1, distorted * 0.5 + 0.5);
        }
    )";

    static const char* FISH_EYE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uStrength = 0.25; // Distortion strength (default: 1.0)
        out vec4 FragColor;

        void main() {
            vec2 centered = TexCoord * 2.0 - 1.0;
            float radius = length(centered);
            vec2 distorted = centered / (1.0 + uStrength * radius);
            FragColor = texture(texture1, distorted * 0.5 + 0.5);
        }
    )";

    static const char* ASCII_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 _Resolution_; // Screen resolution
        uniform int blockSize = 8;   // Block size in pixels (default: 8)
        out vec4 FragColor;

        const float asciiLevels[10] = float[10](0.0, 0.1, 0.2, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0);

        void main() {
            vec2 block = floor(TexCoord * _Resolution_ / blockSize) * blockSize / _Resolution_;
            float intensity = texture(texture1, block).r;
            int index = int(intensity * 10.0);
            FragColor = vec4(vec3(asciiLevels[index]), 1.0);
        }
    )";

    static const char* POLAR_COORDS_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        out vec4 FragColor;

        void main() {
            vec2 centered = TexCoord * 2.0 - 1.0;
            float radius = length(centered);
            float angle = atan(centered.y, centered.x) / 3.14159265;
            FragColor = texture(texture1, vec2(angle, radius));
        }
    )";

    static const char* DOUBLE_VISION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform vec2 offset = vec2(0.01f); // Offset between the two images (default: vec2(0.01, 0.01))
        out vec4 FragColor;

        void main() {
            vec4 original = texture(texture1, TexCoord);
            vec4 duplicate = texture(texture1, TexCoord + offset);
            FragColor = mix(original, duplicate, 0.5);
        }
    )";

    static const char* SHARPEN_SHADER = R"(
        in vec2 TexCoord;
        out vec4 FragColor;

        uniform sampler2D uTexture;  // Input texture
        uniform vec2 _Resolution_; // Size of a single texel (1.0/width, 1.0/height)
        uniform float uSharpness; // Sharpness intensity (range: 0.0 to 2.0)

        void main()
        {
            // Base convolution kernel for sharpening
            float kernel[9] = float[](
                0.0, -1.0,  0.0,
               -1.0,  5.0, -1.0,
                0.0, -1.0,  0.0
            );

            // Offsets for surrounding texels
            vec2 offsets[9] = vec2[](
                vec2(-1,  1) / _Resolution_,  // Top-left
                vec2( 0,  1) / _Resolution_,  // Top-center
                vec2( 1,  1) / _Resolution_,  // Top-right
                vec2(-1,  0) / _Resolution_,  // Mid-left
                vec2( 0,  0) / _Resolution_,  // Center
                vec2( 1,  0) / _Resolution_,  // Mid-right
                vec2(-1, -1) / _Resolution_,  // Bottom-left
                vec2( 0, -1) / _Resolution_,  // Bottom-center
                vec2( 1, -1) / _Resolution_   // Bottom-right
            );

            // Perform convolution
            vec4 color = vec4(0.0);
            for (int i = 0; i < 9; i++)
            {
                color += texture(uTexture, TexCoord + offsets[i]) * kernel[i];
            }

            // Blend between the original and sharpened image using uSharpness
            vec4 original = texture(uTexture, TexCoord);
            FragColor = mix(original, color, uSharpness);
        }

    )";

    static const char* SNOW_FALL_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uTime = 0.5;
        out vec4 FragColor;

        void main() {
            float snow = fract(sin(dot(TexCoord * 10.0, vec2(12.9898, 78.233)) + uTime) * 43758.5453123);
            FragColor = mix(texture(texture1, TexCoord), vec4(1.0), step(0.9, snow));
        }
    )";

    static const char* HEXAGONAL_PIXEL_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float uSize = 0.015; // Size of the hexagons (default: 0.1)
        out vec4 FragColor;

        void main() {
            vec2 hex = TexCoord / uSize;
            vec2 q = vec2(hex.x - hex.y / 2.0, hex.y);
            q = floor(q + 0.5) - mod(floor(q), 2.0) / 2.0;
            vec2 uv = vec2(q.x + q.y / 2.0, q.y) * uSize;
            FragColor = texture(texture1, uv);
        }
    )";

    static const char* DOOR_TRANSPARENCY_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;
        uniform float alpha = 0.5; // Transparency (default: 0.5)
        out vec4 FragColor;

        void main() {
            float threshold = fract(gl_FragCoord.x + gl_FragCoord.y * 0.5);
            vec4 color = texture(texture1, TexCoord);
            FragColor = mix(color, vec4(0.0), step(threshold, alpha));
        }
    )";

    static const char* TOON_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform int uLevels = 5; // Number of toon levels (default: 5)

        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            float factor = 1.0 / float(uLevels);
            color.rgb = floor(color.rgb / factor) * factor; // Quantize colors
            FragColor = color;
        }
    )";

    static const char* XRAY_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture

        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114)); // Grayscale
            vec3 xrayColor = vec3(1.0 - luminance, luminance, 1.0 - luminance); // Inverted grayscale
            FragColor = vec4(xrayColor, 1.0);
        }
    )";

    static const char* NEON_GLOW_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform vec3 uGlowColor = vec3(0, 1, 0.5); // Glow color (default: vec3(0.0, 1.0, 0.5))

        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            float intensity = length(color.rgb); // Intensity based on brightness
            vec3 neonEffect = uGlowColor * intensity * 2.0;
            FragColor = vec4(color.rgb + neonEffect, color.a);
        }
    )";

    static const char* NIGHT_VISION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform vec4 uNightVisionColor = vec4(0.1, 1.0, 0.1, 1); // Night vision tint (default: vec3(0.1, 1.0, 0.1))

        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114)); // Grayscale
            FragColor = uNightVisionColor * luminance * 2.0;
        }
    )";

    static const char* EXPLOSION_DISTORTION_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform vec2 uCenter = vec2(0.5, 0.5); // Explosion center (default: vec2(0.5, 0.5))
        uniform float uTime = 0.5; // Animated time
        uniform float uIntensity = 0.2; // Distortion intensity (default: 0.2)

        out vec4 FragColor;

        void main() {
            vec2 uv = TexCoord;
            vec2 dir = uv - uCenter;
            float dist = length(dir);
            float distortion = sin(dist * 10.0 - uTime * 5.0) * uIntensity;
            uv += normalize(dir) * distortion;
            FragColor = texture(texture1, uv);
        }
    )";

    static const char* CONTRAST_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform float uContrast;     // Contrast adjustment factor (1.0 = no change, >1.0 = higher contrast, <1.0 = lower contrast)
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);

            // Adjust contrast
            vec3 midpoint = vec3(0.5); // Midpoint for contrast adjustment
            vec3 adjusted = (color.rgb - midpoint) * uContrast + midpoint;

            FragColor = vec4(adjusted, color.a); // Preserve original alpha
        }
    )";

    static const char* BRIGHTNESS_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1; // Input texture
        uniform float uBrightness; 
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);
            color.rgb += uBrightness;
            color.rgb = clamp(color.rgb, 0.0, 1.0);
            FragColor = color; // Preserve original alpha
        }
    )";

    static const char* TEMPERATURE_SHADER = R"(
        in vec2 TexCoord;
        out vec4 FragColor;

        uniform sampler2D uTexture;
        uniform float uTemperature; // Temperature adjustment (-1.0 = cooler, 0.0 = no change, 1.0 = warmer)

        void main()
        {
            vec4 color = texture(uTexture, TexCoord);
            color.r += uTemperature * 0.1; // Add warmth (red channel)
            color.b -= uTemperature * 0.1; // Remove coolness (blue channel)
            FragColor = vec4(color.rgb, color.a); // Preserve alpha
        }
    )";

    static const char* GAMMA_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;  // Input texture
        uniform float uGamma;         // Gamma value (e.g., 2.2 for sRGB correction)
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);

            // Apply gamma correction
            vec3 gammaCorrected = pow(color.rgb, vec3(1.0 / uGamma));
            FragColor = vec4(gammaCorrected, color.a);
        }
    )";

    static const char* VIGNETTE_SHADER = R"(
        in vec2 TexCoord;
        uniform sampler2D texture1;  // Input texture
        uniform vec2 _Resolution_;     // Resolution of the screen or texture
        uniform float uVignetteStrength; // Strength of vignette effect (e.g., 0.5 for subtle, 1.0 for stronger)
        out vec4 FragColor;

        void main() {
            vec4 color = texture(texture1, TexCoord);

            // Calculate distance from the center
            vec2 position = TexCoord * _Resolution_;
            vec2 center = _Resolution_ * 0.5;
            float distance = length((position - center) / _Resolution_);

            // Apply vignette effect
            float vignette = smoothstep(0.5, 0.9, 1.0 - uVignetteStrength * distance);
            vec3 result = color.rgb * vignette;

            FragColor = vec4(result, color.a);
        }
    )";
}
