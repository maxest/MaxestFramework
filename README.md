# MaxestFramework

My C++ framework on which I experiment with various things, mostly pertaining to real-time rendering. It features a bunch of real-time rendering demos implemented using Direct3D 11. The demos depict effects like temporal anti-aliasing, screen-space ambient occlusion and my novel contact-hardening soft shadows implementation, among other things.

## Selected Demos

### Shadows

Contact-hardening soft shadows implementation based on [my article](https://wojtsterna.files.wordpress.com/2023/02/contact_hardening_soft_shadows.pdf).

![image](https://user-images.githubusercontent.com/37375338/217040404-d3cad935-dc43-4845-a7c7-69d44d69598d.png)

### Volumetrics

Volumetric fog based on [this publication](https://bartwronski.com/2014/08/11/siggraph-2014-talk-slides-are-up/).

![image](https://user-images.githubusercontent.com/37375338/217041461-0d288a54-f487-4b39-88e6-39da2cf28b54.png)

### Screen Space Ambient Occlusion

SSAO implementation based on my chapter from [this book](https://www.amazon.com/GPU-Zen-Advanced-Rendering-Techniques-ebook/dp/B0711SD1DW).

![image](https://user-images.githubusercontent.com/37375338/217041846-93c1f848-cb87-498b-8620-d4ffe36b9656.png)

### Depth of Field

DoF implementation based on my chapter from [this book](https://www.amazon.com/GPU-Zen-Advanced-Rendering-Techniques-ebook/dp/B0711SD1DW).

![image](https://user-images.githubusercontent.com/37375338/217045594-9d2ad657-195b-4186-ac23-57b9c2c5dfc3.png)

### Antialiasing

Three AA techniques supported:
1. Temporal antialiasing blending two frames. 
2. Simple edge blur.
3. FXAA.

![image](https://user-images.githubusercontent.com/37375338/217042794-3d127657-8237-4f61-8ff0-37332300b46c.png)

### Software Ray Tracer

Simple software ray tracer. Uses the framework's job system to render the image faster.

![image](https://user-images.githubusercontent.com/37375338/217048742-d6e07306-3b47-4369-91fa-d862864bcf29.png)

### Fourier

Calculates Discrete Fourier Transform (DFT). It does not use Fast Fourier Transform (FFT), so the implementation is very simple. It leverages separability though to speed up processing by an order of magnitude (in relation to plain DFT).

![image](https://user-images.githubusercontent.com/37375338/217050855-883c8422-d308-4d79-8102-218ac57de6f2.png)
![image](https://user-images.githubusercontent.com/37375338/217050886-1cc4582b-29c2-4c0d-b6c8-69041e6ff279.png)
