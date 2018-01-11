#version 330 core

// interpolated entry position for a ray through the volume
in vec3 entryPos;

// out location 0 is piped directly to the default draw buffer
out vec4 outColor;

// uniforms use the same value for all fragments
uniform sampler1D transferFunction; // to map sampled intensities to color
uniform sampler2D exitPositions; // precalculated exit positions for an orthogonal ray from each fragment
uniform sampler3D volume;
//uniform sampler3D gradients; // directions of greatest change at each voxel, used as normals for shading
uniform int numSamples; // number of samples along each ray
uniform float sampleRangeStart; // skip samples up to this point
uniform float sampleRangeEnd; // skip samples after this point
uniform float shadingThreshold;
uniform vec2 screenDimensions;
uniform float opacityFactor;
uniform float opacityOffset;
uniform float ttfSampleFactor; // multiply transfer function texture sample position
uniform float ttfSampleOffset; // offset transfer function texture sample position
uniform float midaParam; // in range [-1,1]

// COMPOSITING METHODS
// 0: Alpha compositing ("DVR")
// 1: Maximum Intensity Difference Accumulation
// 2: Maximum Intensity Projection
// 3: Average Intensity Projection
uniform int compositingMethod;
uniform bool enableShading;

void main()
{

    vec3 exitPos = texture(exitPositions, gl_FragCoord.st/screenDimensions).xyz;

    if (entryPos == exitPos) {
        discard;
    }

    vec3  ray = exitPos - entryPos;
    float sampleStepSize = length(ray)/numSamples;
    vec3  rayDelta = normalize(ray) * sampleStepSize;
    vec3  currentVoxelPos = entryPos;

    // Shading
    vec3  view = vec3(0, 0, 10); // view vector pointing to camera
    vec3  firstHitPos = vec3(0); // first hit voxel position

    vec3 lightPos = vec3(5, 5, 5);
    vec3 lightAmb = vec3(0.7);
    vec3 lightDif = vec3(0.7);
    vec3 lightSpec = vec3(0.6);

    float intensity = 0.0;
    float minIntensity = 1.0;
    float maxIntensity = 0.0;
    float intensityAccum = 0.0;
    float intensityCount = 0.0;
    vec4  mappedColor; // color mapped to intensity by transferFunction
    vec4  colorAccum = vec4(0.0); // accumulated color from volume traversal

    vec4 backgroundColor = vec4(1.0, 1.0, 1.0, 0.0);

    for (int i = 0; i < numSamples; ++i) {

        if (i >= sampleRangeStart * numSamples && i <= sampleRangeEnd * numSamples) {

            intensity = texture(volume, currentVoxelPos).r;

            if (firstHitPos == vec3(0) && intensity > shadingThreshold) {
                firstHitPos = currentVoxelPos;
            }


            if (compositingMethod == 0) { // ALPHA COMPOSITING

                mappedColor = texture(transferFunction, intensity * ttfSampleFactor + ttfSampleOffset);
                mappedColor.a = intensity * opacityFactor + opacityOffset; // alpha is opacity, i.e. occlusion

                // how much of a voxel mappedColor shines through depends on its own opacity mappedColor.a
                // and how much transparency (1 - colorAccum.a) is left to viewer after accumulation of opacity colorAccum.a
                colorAccum.rgb = colorAccum.rgb
                               + (1 - colorAccum.a) * mappedColor.a * mappedColor.rgb;
                colorAccum.a = colorAccum.a
                             + (1 - colorAccum.a) * mappedColor.a;

                if (colorAccum.a > 1.0) {
                    colorAccum.a = 1.0;
                    break; // terminate if accumulated opacity > 1
                }
            }

            if (compositingMethod == 1) { // MAXIMUM INTENSITY DIFFERENCE ACCUMULATION

                // the traditional alpha compositing method here is referred to under broad term of DVR as in literature
                //
                // DVR:
                // accumulated color = prev. acc. color
                //                   + (1 - prev. acc. opacity) * opacity * color
                // accumulated opacity = prev. acc. opacity
                //                     + (1 - prev. acc. opacity) * opacity
                // note that (1 - prev. acc. opacity) is the transparency left for current voxel to shine through
                //
                // MIDA:
                // accumulated color = (1 - weight) * prev. acc. color
                //                   + (1 - (1 - weight) * prev. acc. opacity) * opacity * color
                // accumulated opacity = (1 - weight) * prev. acc. opacity
                //                     + (1 - (1 - weight) * prev. acc. opacity) * opacity
                // (1 - (1 - weight) * prev. acc. opacity) is a weighted transparency,
                // higher weight -> lower previous opacity -> higher transparency for current voxel to shine through
                //
                // weight = if new max intensity at current position then (new max - old max), else 0.
                //
                // idea of the weight: when the maximum changes to a new higher value, it likely means
                // we have a new important structure, thus the corresponding sample should have more weight.
                // if intensity is constant we dont do accumulation, to avoid occlusion e.g. from thick irrelevant medium
                // changes should be more important than constant intensity.
                // if weight = 1, the previously accumulated color is ignored and we have local MIP
                // if weight = 0, only the previously accumulated color counts and current color is ignored.
                // => the higher the weight, the less accumulation matters and the more the current value counts.
                // thus even if opacity was already accumulated to almost 1,
                // if new maximum, previous accumulation is weighted less, leaving more transparency for new to shine through,
                // thereby also lowering the relative contribution of what came before.
                // usually weight will be inbetween, thus giving the advantage of
                // important structures shining through as in MIP combined with depth cue from some accumulation


                mappedColor = texture(transferFunction, intensity * ttfSampleFactor + ttfSampleOffset);
                mappedColor.a = intensity * opacityFactor + opacityOffset; // alpha is opacity, i.e. occlusion

                float weight = 0;
                if (intensity > maxIntensity) {
                    weight = intensity - maxIntensity;
                    maxIntensity = intensity;
                }

                // midaParam is in range [-1,1] and used to interpolated between using DVR, MIDA and MIP
                // if it is -1, the weight is 0, thus leading to DVR
                // if it is 0, the weight is just the weight, i.e. MIDA
                // if it is 1, instead of changing the weight we later interpolate resulting colors between MIDA and max value
                if (midaParam < 0) {
                    weight = weight*(1 + midaParam);
                }

                // how much of a voxel mappedColor shines through depends on its own opacity mappedColor.a
                // and how much transparency (1 - colorAccum.a) is left to viewer after accumulation of opacity colorAccum.a
                colorAccum.rgb = (1 - weight) * colorAccum.rgb
                               + (1 - (1 - weight) * colorAccum.a) * mappedColor.a * mappedColor.rgb;
                colorAccum.a = (1 - weight) * colorAccum.a
                             + (1 - (1 - weight) * colorAccum.a) * mappedColor.a;

                if (colorAccum.a > 1.0) {
                    colorAccum.a = 1.0;
                    break; // terminate if accumulated opacity > 1
                }
            }

            else if (compositingMethod == 2) { // MAXIMUM INTENSITY PROJECTION
                if (intensity > maxIntensity) {
                    maxIntensity = intensity;
                }
            }

            else if (compositingMethod == 3) { // AVERAGE INTENSITY PROJECTION
                intensityAccum += intensity;
                if (intensity > 0.0) {
                    intensityCount += 1;
                }
            }

            else if (compositingMethod == 4) { // MINIMUM INTENSITY PROJECTION
                if (intensity < minIntensity) {
                    minIntensity = intensity;
                }
            }

        }

        currentVoxelPos += rayDelta;
    }

    if (compositingMethod == 0) { // ALPHA COMPOSITING
        outColor = colorAccum;
    }
    else if (compositingMethod == 1) { // MIDA COMPOSITING

        // interpolate resulting colors between MIDA and max value (MIP)
        if (midaParam > 0) {
            vec4 maxColor = texture(transferFunction, maxIntensity * ttfSampleFactor + ttfSampleOffset);
            outColor = midaParam * maxColor + (1 - midaParam) * colorAccum;
        }
        else {
            outColor = colorAccum;
        }
    }
    else if (compositingMethod == 2) { // MAXIMUM INTENSITY PROJECTION
        outColor = texture(transferFunction, maxIntensity * ttfSampleFactor + ttfSampleOffset);
    }
    else if (compositingMethod == 3) { // AVERAGE INTENSITY PROJECTION
        intensityCount = intensityCount > 0.0 ? intensityCount : numSamples;
        float avgIntensity = intensityAccum / intensityCount;
        avgIntensity = min(avgIntensity, 1.0);
        outColor = texture(transferFunction, avgIntensity * ttfSampleFactor + ttfSampleOffset);
    }
    else if (compositingMethod == 4) { // MINIMUM INTENSITY PROJECTION
        outColor = texture(transferFunction, minIntensity * ttfSampleFactor + ttfSampleOffset);
    }


    // SHADING VIA BLINN PHONG ILLUMINATION MODEL
    if (enableShading) {

        // approx. surface gradient at current voxel pos
        vec3 gradient;
        gradient.x = texture(volume, vec3(firstHitPos.x+sampleStepSize, firstHitPos.yz)).r - texture(volume, vec3(firstHitPos.x-sampleStepSize, firstHitPos.yz)).r;
        gradient.y = texture(volume, vec3(firstHitPos.x, firstHitPos.y+sampleStepSize, firstHitPos.z)).r - texture(volume, vec3(firstHitPos.x, firstHitPos.y-sampleStepSize, firstHitPos.z)).r;
        gradient.z = texture(volume, vec3(firstHitPos.xy, firstHitPos.z+sampleStepSize)).r - texture(volume, vec3(firstHitPos.xy, firstHitPos.z-sampleStepSize)).r;
        float gradientMagnitude = length(gradient);
        vec3 normal = normalize(gradient);

        // blinn-phong
        vec3 lightDir = normalize(lightPos - firstHitPos);
        vec3 ambient = lightAmb;
        vec3 diffuse = max(dot(normal, lightDir), 0.0f) * lightDif;
        float shininess = 3.f;
        vec3 halfVec = normalize(lightDir + view); // half vector of light and view vectors
        vec3 specular = pow(max(dot(halfVec, normal), 0.0f), shininess) * lightSpec;

        vec3 unshadedColor = outColor.rgb;
        vec3 shadedColor = vec3(ambient + diffuse + specular) * outColor.rgb;

        // weight contribution of shading based on gradient magnitude to avoid applying shading to noise
        float shadingWeight = gradientMagnitude + (1 - gradientMagnitude)/2;
        outColor = vec4(shadingWeight * shadedColor + (1 - shadingWeight) * unshadedColor, outColor.a);

    }

    // DEBUG DRAW FRONT FACES (RAY ENTRY POSITIONS) / BACK FACES (RAY EXIT POSITIONS
    //outColor = vec4(entryPos, 1.0);
    //outColor = vec4(exitPos, 1.0);

}
