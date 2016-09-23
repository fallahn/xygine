/*
Implements screen space water a outlined by Wojciech Toman 
- http://www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/rendering-water-as-a-post-process-effect-r2642
*/

"#if defined(WATER)\n"

"float saturate(float val)\n"
"{\n"
"    return clamp(val, 0.0, 1.0);\n"
"}\n"

"vec3 saturate(vec3 val)"
"{"
"    return clamp(val, vec3(0.0), vec3(1.0));"
"}"

"const float R0 = 0.5;\n"
"const float refractionStrength = 0.0;\n"
"float calcFresnel(vec3 normal, vec3 eyeVec)\n"
"{\n"
"    float angle = 1.0 - max(0.0, dot(normal, eyeVec));\n"
"    float fresnel = angle * angle;\n"
"    fresnel = fresnel * fresnel;\n"
"    fresnel = fresnel * angle;\n"
"    return saturate(fresnel * (1.0 - saturate(R0)) + R0 - refractionStrength);\n"
"}\n"

"mat3 calcTangentFrame(vec3 N, vec3 P, vec2 UV)\n"
"{\n"
"    vec3 dp1 = dFdx(P);\n"
"    vec3 dp2 = dFdy(P);\n"
"    vec2 duv1 = dFdx(UV);\n"
"    vec2 duv2 = dFdy(UV);\n"

"    vec3 dp1xdp2 = cross(dp1, dp2);\n"
"    mat2x3 inverseM = mat2x3(cross(dp2, dp1xdp2), cross(dp1xdp2, dp1));\n"

"    vec3 T = inverseM * vec2(duv1.x, duv2.x);\n"
"    vec3 B = inverseM * vec2(duv1.y, duv2.y);\n"

"    float maxLength = max(length(T), length(B));\n"
"    T = T / maxLength;\n"
"    B = B / maxLength;\n"

"    return mat3(T, B, N);\n"
"}\n"

"//vec2 frustumNearFar = vec2(-100.0, 600.0);\n"
"const float LOG2 = 1.442695;\n"

"uniform float u_waterLevel = 480.0;\n"
"uniform vec4 u_waterColour = vec4(0.0078, 0.517, 0.7, 1.0);\n"
"uniform vec4 u_deepWaterColour = vec4(0.0039, 0.00196, 0.145, 1.0);\n"
"uniform float u_transparency = 0.03;\n"

"vec3 calcUnderWater(vec3 colour, vec3 fragPosition)\n"
"{\n"
"    if(u_skyLight.intensity > 0)\n"
"    {\n"
"        float depth = fragPosition.z;\n"
"        vec3 eyeVec = fragPosition - u_cameraWorldPosition;\n"

"        //get surface intersection\n"
"        vec3 eyeVecNorm = normalize(eyeVec);\n"
"        float t = (u_waterLevel - u_cameraWorldPosition.y) / eyeVecNorm.y;\n"
"        vec3 surfacePoint = u_cameraWorldPosition + eyeVecNorm * t;\n"
"        float cameraDepth = length(u_cameraWorldPosition - surfacePoint);\n"

"        //TODO calc surface normals from tex lookup"
"        eyeVecNorm = normalize(u_cameraWorldPosition - surfacePoint);\n"
"        vec3 normal = vec3(0.0, -1.0, 0.0); //negate as we're below the surface\n"
"        float fresnel = calcFresnel(normal, eyeVecNorm);\n"
"        //TODO offset refraction lookup\n"
"        vec3 refraction = colour;\n"
"        float waterColour = saturate(length(u_skyLight.diffuseColour.rgb) / (u_skyLight.intensity * 500.0));\n"
"        refraction = mix(mix(refraction, u_deepWaterColour.rgb * waterColour, u_transparency),  u_waterColour.rgb * waterColour, u_transparency);\n"

"        if(fragPosition.y < u_waterLevel)\n"
"        {\n"
"            //draw frag as viewed through surface\n"
"            //TODO specularity\n"
"            float fogIntensity = u_transparency * 8.0;\n"
"            float fogAmount = saturate(exp2(-fogIntensity * fogIntensity * cameraDepth * 0.03 * LOG2));\n"

"            colour = mix(u_deepWaterColour.rgb, refraction, fogAmount);\n"
"        }\n"
"        else\n"
"        {\n"
"            //TODO some caustic effect?\n"
"            //float fogDepth = (2.0 * frustumNearFar.x) / (frustumNearFar.y + frustumNearFar.x - depth * (frustumNearFar.y - frustumNearFar.x));\n"
"            float fogIntensity = u_transparency * 8.0;\n"
"            //float fogAmount = clamp((depth / 300.0), 0.0, 0.47);\n"
"            //float fogAmount = saturate(exp2(-fogIntensity * fogIntensity * fogDepth * fogDepth * LOG2));\n"
"            float fogAmount = saturate(exp2(-fogIntensity * fogIntensity * cameraDepth * 0.07 * LOG2));\n"
"            colour = mix(u_deepWaterColour.rgb, colour, fogAmount);\n"
"        }\n"

"        return colour;\n"
"    }\n"
"    else\n"
"    {\n"
"        //no light so it's dark...\n"
"        return colour *= 0.1;\n"
"    }\n"
"}\n"

"const float visibility = 3.0;\n"
"vec3 calcWater(vec3 colour, vec3 fragPosition)\n"
"{\n"
"    if(u_waterLevel > u_cameraWorldPosition.y)\n"
"    {\n"
"        return calcUnderWater(colour, fragPosition);\n"
"    }\n"

"    return vec3(0.0, 1.0, 0.0);\n"
"}\n"

"#endif\n"
