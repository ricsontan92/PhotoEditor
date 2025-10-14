#include "UIFilters.h"

#define REGISTER_FILTER(name, shader) filtersMap[name] = LibGraphics::TextureFilter::CreateFromShader(shader);

#define REGISTER_UNIFORM_VARIABLE_INT(shaderName, uniformName, defaultValue, minValue, maxValue) \
    filtersMap[shaderName]->SetInt(uniformName, defaultValue);\
    shaderVars_Ints[shaderName][uniformName].MinValue = minValue;\
    shaderVars_Ints[shaderName][uniformName].MaxValue = maxValue;\
    shaderVars_Ints[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_FLOAT(shaderName, uniformName, defaultValue, minValue, maxValue) \
    filtersMap[shaderName]->SetFloat(uniformName, defaultValue);\
    shaderVars_Floats[shaderName][uniformName].MinValue = minValue;\
    shaderVars_Floats[shaderName][uniformName].MaxValue = maxValue;\
    shaderVars_Floats[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2(shaderName, uniformName, defaultValue, minValue, maxValue) \
    filtersMap[shaderName]->SetVec2(uniformName, defaultValue);\
    shaderVars_FloatVec2s[shaderName][uniformName].MinValue = minValue;\
    shaderVars_FloatVec2s[shaderName][uniformName].MaxValue = maxValue;\
    shaderVars_FloatVec2s[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_FLOAT_VEC3(shaderName, uniformName, defaultValue, minValue, maxValue) \
    filtersMap[shaderName]->SetVec3(uniformName, defaultValue);\
    shaderVars_FloatVec3s[shaderName][uniformName].IsColor = false;\
    shaderVars_FloatVec3s[shaderName][uniformName].MinValue = minValue;\
    shaderVars_FloatVec3s[shaderName][uniformName].MaxValue = maxValue;\
    shaderVars_FloatVec3s[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_FLOAT_VEC4(shaderName, uniformName, defaultValue, minValue, maxValue) \
    filtersMap[shaderName]->SetVec4(uniformName, defaultValue);\
    shaderVars_FloatVec4s[shaderName][uniformName].IsColor = false;\
    shaderVars_FloatVec4s[shaderName][uniformName].MinValue = minValue;\
    shaderVars_FloatVec4s[shaderName][uniformName].MaxValue = maxValue;\
    shaderVars_FloatVec4s[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_COLOR3(shaderName, uniformName, defaultValue) \
    filtersMap[shaderName]->SetVec3(uniformName, defaultValue);\
    shaderVars_FloatVec3s[shaderName][uniformName].IsColor = true;\
    shaderVars_FloatVec3s[shaderName][uniformName].DefaultValue = defaultValue;

#define REGISTER_UNIFORM_VARIABLE_COLOR4(shaderName, uniformName, defaultValue) \
    filtersMap[shaderName]->SetVec4(uniformName, defaultValue);\
    shaderVars_FloatVec4s[shaderName][uniformName].IsColor = true;\
    shaderVars_FloatVec4s[shaderName][uniformName].DefaultValue = defaultValue;

UIFilters::UIFilters(
    const std::shared_ptr<PanelSharedData>& sharedData,
    const std::shared_ptr<ImageProcessor>& imageProcessor)
	: UIHeader{ sharedData }
    , imageProcessor{ imageProcessor }
    , filterSelectName{ "" }
    , filtersMap{}
{}

void UIFilters::Init()
{
    REGISTER_FILTER("Bloom", LibGraphics::BLOOM_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Bloom", "uThreshold", 0.05f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Blur(Guassian)", LibGraphics::GAUSSIAN_BLUR_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Blur(Guassian)", "uBlurScale", 1.0f, 1.0f, 10.0f);
    }

    REGISTER_FILTER("Blur(Radial)", LibGraphics::RADIAL_BLUR_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_INT("Blur(Radial)", "uBlurSteps", 10, 5, 50);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Blur(Radial)", "uBlurStrength", 0.05f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Blur(Radial)", "uBlurCenter", LibCore::Math::Vec2(0.5f, 0.5f), 0.0f, 1.0f);
    }

    REGISTER_FILTER("Outline", LibGraphics::OUTLINE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Outline", "uThreshold", 0.05f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Chrome", LibGraphics::CHROMATIC_ABERRATION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Chrome", "uIntensity", 0.25f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Grayscale", LibGraphics::GRAY_SCALE_SHADER);
    {

    }

    REGISTER_FILTER("Edge Detect(Sobel)", LibGraphics::SOBEL_EDGE_DETECT_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Edge Detect(Sobel)", "uThreshold", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Edge Detect(Prewitt)", LibGraphics::PREWITT_EDGE_DETECT_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Edge Detect(Prewitt)", "uThreshold", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Edge Detect(Roberts Cross)", LibGraphics::ROBERTS_CROSS_EDGE_DETECT_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Edge Detect(Roberts Cross)", "uThreshold", 0.5f, 0.0f, 1.0f);
    }

    filtersMap["Edge Detect(Canny)"] = LibGraphics::TextureFilter::CreateFromShaders(
        {
            LibGraphics::CANNY_EDGE_DETECT_BLUR_SHADER,
            LibGraphics::CANNY_EDGE_DETECT_SOBEL_SHADER,
            LibGraphics::CANNY_EDGE_DETECT_THRESHOLD_SHADER,
            LibGraphics::CANNY_EDGE_DETECT_HYSTERIESIS_SHADER,
        }
    );
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Edge Detect(Canny)", "uEdgeThresholdLow", 0.1f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Edge Detect(Canny)", "uEdgeThresholdHigh", 0.3f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Sepia", LibGraphics::SEPIA_TONE_SHADER);
    {

    }

    REGISTER_FILTER("Distortion", LibGraphics::DISTORTION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Distortion", "uTime", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Posterisation", LibGraphics::POSTERISATION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_INT("Posterisation", "uLevels", 5, 1, 10);
    }

    REGISTER_FILTER("Noise", LibGraphics::NOISE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Noise", "uNoiseAmount", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Snow Fall", LibGraphics::SNOW_FALL_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Snow Fall", "uTime", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Hex Pixelation", LibGraphics::HEXAGONAL_PIXEL_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Hex Pixelation", "uSize", 0.015f, 0.0f, 0.1f);
    }

    REGISTER_FILTER("Half Tone", LibGraphics::HALF_TONE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Half Tone", "uDotSize", 5, 1, 50);
    }

    REGISTER_FILTER("VHS", LibGraphics::VHS_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("VHS", "uTime", 0.5, 0, 1);
    }

    REGISTER_FILTER("Toon", LibGraphics::TOON_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_INT("Toon", "uLevels", 5, 1, 10);
    }

    REGISTER_FILTER("Gods Ray", LibGraphics::GODS_RAY_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Gods Ray", "uIntensity", 1.0f, 0.0f, 5.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Gods Ray", "uSize", 2.5f, 1.0f, 10.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Gods Ray", "uLightPosition", LibCore::Math::Vec2(0.5f, 0.5f), 0.0f, 1.0f);
    }

    REGISTER_FILTER("Emboss", LibGraphics::EMBOSS_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Emboss", "uTexelSize", LibCore::Math::Vec2(1.0f, 1.0f), 1.0f, 100.0f);
    }

    REGISTER_FILTER("Mosiac", LibGraphics::MOSIAC_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Mosiac", "uMosaicSize", 5.0f, 1.0f, 100.0f);
    }

    REGISTER_FILTER("Swirling", LibGraphics::SWIRLING_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Swirling", "uTime", 0.5f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Swirling", "uStrength", 5.0f, 1.0f, 100.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Swirling", "uCenter", LibCore::Math::Vec2(0.5f, 0.5f), 0.0f, 1.0f);
    }

    REGISTER_FILTER("Gradient Overlay", LibGraphics::GRADIENT_OVERLAY_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_COLOR4("Gradient Overlay", "uColor1", LibCore::Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
        REGISTER_UNIFORM_VARIABLE_COLOR4("Gradient Overlay", "uColor2", LibCore::Math::Vec4(1.0f, 0.0f, 1.0f, 1.0f));
    }
    
    REGISTER_FILTER("Glitch Lines", LibGraphics::GLITCH_LINES_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Glitch Lines", "uTime", 0.5f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Glitch Lines", "uGlitchSize", 0.1f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Ripple", LibGraphics::DYNAMIC_RIPPLE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Ripple", "uTime", 0.5f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Ripple", "uAmplitude", 0.05f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Ripple", "uFrequency", 10.0f, 1.0f, 100.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Ripple", "uCenter", LibCore::Math::Vec2(0.5f, 0.5f), 0.0f, 1.0f);
    }

    REGISTER_FILTER("Lens Distort", LibGraphics::LENS_DISTORTION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Lens Distort", "uDistortion", 0.5f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Neon", LibGraphics::NEON_GLOW_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_COLOR3("Neon", "uGlowColor", LibCore::Math::Vec3( 0.0f, 1.0f, 0.5f ));
    }

    REGISTER_FILTER("Night Vision", LibGraphics::NIGHT_VISION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_COLOR4("Night Vision", "uNightVisionColor", LibCore::Math::Vec4(0.1f, 1.0f, 0.1f, 1.0f));
    }

    REGISTER_FILTER("Fish Eye", LibGraphics::FISH_EYE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Fish Eye", "uStrength", 0.25f, 0.0f, 1.0f);
    }

    REGISTER_FILTER("Ex Distortion", LibGraphics::EXPLOSION_DISTORTION_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Ex Distortion", "uTime", 0.5f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT("Ex Distortion", "uIntensity", 0.25f, 0.0f, 1.0f);
        REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2("Ex Distortion", "uCenter", LibCore::Math::Vec2(0.5f, 0.5f), 0.0f, 1.0f);
    }

    REGISTER_FILTER("X-Ray", LibGraphics::XRAY_SHADER);
    {

    }

    REGISTER_FILTER("Vignette", LibGraphics::VIGNETTE_SHADER);
    {
        REGISTER_UNIFORM_VARIABLE_FLOAT("Vignette", "uVignetteStrength", 0.5f, 0.0f, 1.0f);
    }

    filterSelectName = filtersMap.begin()->first;
}

void UIFilters::Render(float dt)
{
    std::string comboStr = "";
    for (auto& filter : filtersMap)
    {
        comboStr += filter.first;
        comboStr.push_back(0);
    }
    comboStr.push_back(0);

    int filterIdx = static_cast<int>(std::distance(filtersMap.begin(), filtersMap.find(filterSelectName)));
    ImGui::BeginChild("##FILTER_LIST", ImVec2{ 0, ImGui::GetContentRegionAvail().y }, ImGuiChildFlags_Border);
    {
        ImGui::PushItemWidth(std::max(10.0f, ImGui::GetContentRegionAvail().x - 30.0f));
        if (ImGui::Combo("##FILTERS_SELECT", &filterIdx, comboStr.c_str()) && filterIdx >= 0 && filterIdx < filtersMap.size())
        {
            auto filterIt = filtersMap.begin();
            std::advance(filterIt, filterIdx);
            filterSelectName = filterIt->first;
        }
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button("+##FILTERS_ADD", ImVec2{ ImGui::GetContentRegionAvail().x, 0 }))
        {
            if (filtersMap.find(filterSelectName) != filtersMap.end())
            {
                imageProcessor->ImageFilters.push_back({});
                imageProcessor->ImageFilters.back().Active = true;
                imageProcessor->ImageFilters.back().Name = filterSelectName;
                imageProcessor->ImageFilters.back().Filter = filtersMap[filterSelectName]->Clone();

                // set default value
                for (auto& vFloat : shaderVars_Floats[filterSelectName])
                    imageProcessor->ImageFilters.back().Filter->SetFloat(vFloat.first.c_str(), vFloat.second.DefaultValue);
                for (auto& vFloatVec2 : shaderVars_FloatVec2s[filterSelectName])
                    imageProcessor->ImageFilters.back().Filter->SetVec2(vFloatVec2.first.c_str(), vFloatVec2.second.DefaultValue);
            }
        }

        ImGui::Separator();


        for (size_t i = 0; i < imageProcessor->ImageFilters.size(); ++i)
        {
            auto& currFilter = imageProcessor->ImageFilters[i];
            std::string imguiIdx = "##FILTER_" + std::to_string((long long)&currFilter) + "_";

            ImGui::Checkbox((imguiIdx + "Active").c_str(), &currFilter.Active);
            ImGui::SameLine();
            bool selRemoveFilter = true;
            if (ImGui::CollapsingHeader((currFilter.Name + imguiIdx + "Details").c_str(), &selRemoveFilter))
            {
                std::string shaderVarIdx = "##" + currFilter.Name + std::to_string((long long)&currFilter) + "_";

                // for ints
                for (auto& vInts : shaderVars_Ints[currFilter.Name])
                {
                    int currValue;
                    if (currFilter.Filter->GetInt(vInts.first.c_str(), currValue))
                    {
                        if (ImGui::DragInt((vInts.first + shaderVarIdx).c_str(), &currValue, 0, vInts.second.MinValue, vInts.second.MaxValue))
                            currFilter.Filter->SetInt(vInts.first.c_str(), currValue);
                    }
                }

                // for floats
                for (auto& vFloat : shaderVars_Floats[currFilter.Name])
                {
                    float currValue;
                    if (currFilter.Filter->GetFloat(vFloat.first.c_str(), currValue))
                    {
                        if (ImGui::DragFloat((vFloat.first + shaderVarIdx).c_str(), &currValue, 0, vFloat.second.MinValue, vFloat.second.MaxValue))
                            currFilter.Filter->SetFloat(vFloat.first.c_str(), currValue);
                    }
                }

                // for floats vec2s
                for (auto& vFloatVec2 : shaderVars_FloatVec2s[currFilter.Name])
                {
                    LibCore::Math::Vec2 currValue;
                    if (currFilter.Filter->GetVec2(vFloatVec2.first.c_str(), currValue))
                    {
                        if (ImGui::DragFloat2((vFloatVec2.first + shaderVarIdx).c_str(), &currValue.x, 0, vFloatVec2.second.MinValue, vFloatVec2.second.MaxValue))
                            currFilter.Filter->SetVec2(vFloatVec2.first.c_str(), currValue);
                    }
                }

                // for floats vec3s
                for (auto& vFloatVec3 : shaderVars_FloatVec3s[currFilter.Name])
                {
                    LibCore::Math::Vec3 currValue;
                    if (currFilter.Filter->GetVec3(vFloatVec3.first.c_str(), currValue))
                    {
                        if (vFloatVec3.second.IsColor)
                        {
                            if (ImGui::ColorPicker3((vFloatVec3.first + shaderVarIdx).c_str(), &currValue.x))
                                currFilter.Filter->SetVec3(vFloatVec3.first.c_str(), currValue);
                        }
                        else
                        {
                            if (ImGui::DragFloat3((vFloatVec3.first + shaderVarIdx).c_str(), &currValue.x, 0, vFloatVec3.second.MinValue, vFloatVec3.second.MaxValue))
                                currFilter.Filter->SetVec3(vFloatVec3.first.c_str(), currValue);
                        }
                    }
                }

                // for floats vec4s
                for (auto& vFloatVec4 : shaderVars_FloatVec4s[currFilter.Name])
                {
                    LibCore::Math::Vec4 currValue;
                    if (currFilter.Filter->GetVec4(vFloatVec4.first.c_str(), currValue))
                    {
                        if (vFloatVec4.second.IsColor)
                        {
                            if (ImGui::ColorPicker4((vFloatVec4.first + shaderVarIdx).c_str(), &currValue.x))
                                currFilter.Filter->SetVec4(vFloatVec4.first.c_str(), currValue);
                        }
                        else
                        {
                            if (ImGui::DragFloat4((vFloatVec4.first + shaderVarIdx).c_str(), &currValue.x, 0, vFloatVec4.second.MinValue, vFloatVec4.second.MaxValue))
                                currFilter.Filter->SetVec4(vFloatVec4.first.c_str(), currValue);
                        }
                    }
                }
            }

            if (!selRemoveFilter)
            {
                imageProcessor->ImageFilters.erase(imageProcessor->ImageFilters.begin() + i);
            }
        }
    }
    ImGui::EndChild();
}

#undef REGISTER_FILTER
#undef REGISTER_UNIFORM_VARIABLE_INT
#undef REGISTER_UNIFORM_VARIABLE_FLOAT
#undef REGISTER_UNIFORM_VARIABLE_FLOAT_VEC2
#undef REGISTER_UNIFORM_VARIABLE_FLOAT_VEC3
#undef REGISTER_UNIFORM_VARIABLE_FLOAT_VEC4
#undef REGISTER_UNIFORM_VARIABLE_COLOR3
#undef REGISTER_UNIFORM_VARIABLE_COLOR4
