# Magnefu Game Engine

## Description

With the 3 pillars of Physics, AI, and Chemistry, the Magnefu Game Engine aims to be a game development tool for creating truly novel gameplay experiences, not just focusing purely on graphics but on interactions with the systems and AI of the game world. The hope is to create a base for truly emergent AI that surprises the player at every turn, physics that mimic(as much as possible) what one would expect and world systems that feel intuitive. For now, only OpenGL is being used but Vulkan and Direct3D graphics APIs will be supported in the future(perhaps Metal as well).

## Currently Supported Platforms
- Windows x64

## Currently Supported Graphics APIs
- OpenGL

## Installation

## Premake

Project files can be generated - only Windows for now - by running the GenerateProjects.bat


## Building

Note that if you receive a message about Magnefu.dll not being a valid win32 application, delete the .vs folder in the root directory and then rerun the GenerateProjects.bat.

## Controls

- Key binds
> ![controls.png](./SCREENS/controls.png)

- Finding Controls
> ![howtocontrols.gif](./SCREENS/howtocontrols.gif)

## Features

### Current 

- OBJ Mesh Loading
- Texture Filtering
    - Bilinear
- BRDF
    - Phong
    - Modified Phong (Energy Conserving)
- Lights
    - Point
    - Spot 
    - Direction

NOTE - Spot and Direction lights are temporarily disabled as I fix some issues with the lighting calculations

- Test Scenes


### Upcoming
- Adaptive Window Resolution / Screen Resolution Options
- BRDF
    - Cook-Torrance Microfacet
    - Blinn-Phong
- Anti Aliasing
- Lights
    - Area
    - Light Attenuation
- Shadows
    - Shadow Mapping
        - Cascaded
        - Moment
- Preset Materials(Non-textured)
- Preset Primitive Meshes
    - Cube
    - Sphere
- Loading of OBJ meshes w/o predefined normals
- Physics
    - Collision




## Test Scenes

These test scenes are here for the purpose of showcasing different rendering techniques from lighting to shading to loading models. As I learn more about the different Graphics APIs and graphics rendering in general,
I will add more of these test scenes as a way of showing my understanding of the theory. Eventually, when the engine is actually underway, they could be a great way for other users to see how different rendering concepts can be implemented.

### Test Clear Color

Just a simple test to change the background color.


### Test 2D Render

![Texturing2DQuad.png](./SCREENS/Texturing2DQuad.png)

### Test 3D Render

![CubeRenderWithTexture.png](./SCREENS/CubeRenderWithTexture.png)

### Test Batching

![QuadBatch.png](./SCREENS/QuadBatch.png)

### Test Light Types

- Lighting Images
> ![TestLighting.png](./SCREENS/TestLighting.png)
> ![TestLighting2.png](./SCREENS/TestLighting2.png)

- Lighting GIFs
> ![PointLight.gif](./SCREENS/PointLight.gif)
> ![SpotLight.gif](./SCREENS/SpotLight.gif)
> ![Cutoff.gif](./SCREENS/Cutoff.gif)
> ![ReflectionModels.gif](./SCREENS/ReflectionModels.gif)

### Test Model Loading

- Lighting temporarily disabled in this scene.

![CatModel.png](./SCREENS/CatModel.png)

![CatModel.gif](./SCREENS/CatModel.gif)

![SantaHat.gif](./SCREENS/SantaHat.gif)

![SantaHatArtifacts.gif](./SCREENS/SantaHatArtifacts.gif)

![LoadModel.gif](./SCREENS/LoadModel.gif)

![UnloadAndLoad.gif](./SCREENS/UnloadAndLoad.gif)

## Incomplete Features


## Known Bugs

- Texture on first model displays correctly. Artifacting of textures on all subsequently loaded models.
