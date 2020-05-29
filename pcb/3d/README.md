# How to build 3D models in EasyEDA

The main source of this article is [EASYEDA3D].

# An example of VRML

Use the example below to follow the procedure:

* Go to the X3D converter:
  * http://doc.instantreality.org/tools/x3d_encoding_converter/
* Paste the code example below.
* Compile it.
* Copy the results into a file called ``test.html``.
* Save the file anywhere.
* Open it with your favourite internet browser.
* See the 3d.

The example below comes from [WEB3DEX] and is VRML97 source code to display a box and a sphere:

``` vrml
#VRML V2.0 utf8
Transform {
  children [
    NavigationInfo { headlight FALSE } # We'll add our own light

    DirectionalLight {        # First child
        direction 0 0 -1      # Light illuminating the scene
    }

    Transform {               # Second child - a red sphere
      translation 3 0 1
      children [
        Shape {
          geometry Sphere { radius 2.3 }
          appearance Appearance {
            material Material { diffuseColor 1 0 0 }   # Red
         }
        }
      ]
    }

    Transform {               # Third child - a blue box
      translation -2.4 .2 1
      rotation     0 1 1  .9
      children [
        Shape {
          geometry Box {}
          appearance Appearance {
            material Material { diffuseColor 0 0 1 }  # Blue
         }
        }
      ]
    }

  ] # end of children for world
}
```

# Useful links

* **X3D encoding converter**
  * You can convert WML files into HTML files that are viewable in any browser.
  * http://doc.instantreality.org/tools/x3d_encoding_converter/

# Bibliography
* [L3D] - **VRML Interactive Tutorial @ Lighthouse 3D**
  * An introduction to VRML, which is rather complete. As VRML is not that modern
  any more, it is not very easy to find good tutorials.
  * http://lighthouse3d.com/vrml/tutorial/index.shtml?intro
* [X3DWA] - **X3D for Web Authors (X3D4WA) Examples Archive**
  * A list of working VRML Examples
  * http://x3dgraphics.com/examples/X3dForWebAuthors/index.html
* [EASYEDA3D] - **Import 3D File**
  * The documentation of EasyEDA on how to import 3D models.
  * https://docs.easyeda.com/en/PCBLib/PCB-3DLib-Import/index.html
* [WEB3DEX] - **The Virtual Reality Modeling Language - Annex D - Examples**
  * More examples.
  * https://www.web3d.org/documents/specifications/14772/V2.0/part1/examples.html
