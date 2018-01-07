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
uniform float opacityOffset;

// COMPOSITING METHODS
// 0: Maximum Intensity Projection
// 1: Average Intensity Projection
// 2: Maximum Intensity Difference Accumulation
// 3: Alpha compositing
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
    vec3  normal = vec3(0);
    vec3  view = vec3(0, 0, 10); // view vector pointing to camera
    vec3  firstHitPos = vec3(0); // first hit voxel position

    vec3 lightPos = vec3(5, 5, 5);
    vec3 lightAmb = vec3(0.7);
    vec3 lightDif = vec3(0.7);
    vec3 lightSpec = vec3(0.6);

    float intensity = 0.0;
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
                //normal = normalize(texture(gradients, currentVoxelPos).rgb);
                firstHitPos = currentVoxelPos;
            }

            if (compositingMethod == 0) { // MAXIMUM INTENSITY PROJECTION
                if (intensity > maxIntensity) {
                    maxIntensity = intensity;
                }
            }
            else if (compositingMethod == 1) { // AVERAGE INTENSITY PROJECTION
                intensityAccum += intensity;
                if (intensity > 0.0) {
                    intensityCount += 1;
                }
            }
            else if (compositingMethod == 2) { // MAXIMUM INTENSITY DIFFERENCE ACCUMULATION

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


                mappedColor = texture(transferFunction, intensity);
                mappedColor.a = intensity + opacityOffset; // alpha is opacity, i.e. occlusion

                float weight = 0;
                if (intensity > maxIntensity) {
                    weight = intensity - maxIntensity;
                    maxIntensity = intensity;
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
            else if (compositingMethod == 3) { // ALPHA COMPOSITING

                mappedColor = texture(transferFunction, intensity);
                mappedColor.a = intensity + opacityOffset; // alpha is opacity, i.e. occlusion

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

        }

        currentVoxelPos += rayDelta;
    }

    if (compositingMethod == 0) { // MAXIMUM INTENSITY PROJECTION
        // <-- toggle transfer function
        outColor = texture(transferFunction, maxIntensity); /*/
        outColor = vec4(vec3(maxIntensity), 1.0); //*/
    }
    else if (compositingMethod == 1) { // AVERAGE INTENSITY PROJECTION
        intensityCount = intensityCount > 0.0 ? intensityCount : numSamples;
        float avgIntensity = intensityAccum / intensityCount;
        if (avgIntensity > 1.0) { avgIntensity = 1.0; }
        outColor = texture(transferFunction, avgIntensity);

    }
    else if (compositingMethod == 2) {  // MIDA COMPOSITING
        outColor = colorAccum;

    }
    else if (compositingMethod == 3) {  // ALPHA COMPOSITING
        outColor = colorAccum;

    }


    // SHADING VIA BLINN PHONG ILLUMINATION MODEL
    if (enableShading) {
        vec3 gradient;
        gradient.x = texture(volume, vec3(firstHitPos.x+sampleStepSize, firstHitPos.yz)).r - texture(volume, vec3(firstHitPos.x-sampleStepSize, firstHitPos.yz)).r;
        gradient.y = texture(volume, vec3(firstHitPos.x, firstHitPos.y+sampleStepSize, firstHitPos.z)).r - texture(volume, vec3(firstHitPos.x, firstHitPos.y-sampleStepSize, firstHitPos.z)).r;
        gradient.z = texture(volume, vec3(firstHitPos.xy, firstHitPos.z+sampleStepSize)).r - texture(volume, vec3(firstHitPos.xy, firstHitPos.z-sampleStepSize)).r;
        normal = normalize(gradient);
        vec3  surfaceColor = outColor.rgb;
        float surfaceAlpha = outColor.a;
        vec3  lightDir = normalize(lightPos - firstHitPos);
        vec3  ambient = lightAmb * surfaceColor;
        vec3  diffuse = max(dot(normal, lightDir), 0.0f) * surfaceColor;
        float shininess = 3.f;

        vec3 halfVec = normalize(lightDir + view); // half vector of light and view vectors
        vec3 specular = pow(max(dot(halfVec, normal), 0.0f), shininess) * lightSpec;

        outColor = vec4(vec3(ambient + diffuse + specular), surfaceAlpha);
    }


    // DEBUG DRAW FRONT FACES (RAY ENTRY POSITIONS) / BACK FACES (RAY EXIT POSITIONS
    //outColor = vec4(entryPos, 1.0);
    //outColor = vec4(exitPos, 1.0);

}
