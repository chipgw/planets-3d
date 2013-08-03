#include "include/common.h"
#include "include/spheregenerator.h"
#include <QVector2D>

Sphere::Sphere(unsigned int slices, unsigned int stacks){
    float vstep = M_PI / stacks;
    float hstep = (2 * M_PI) / slices;

    for(int v = 0; v <= stacks; v++){
        float z = cos(v * vstep);
        float r = sin(v * vstep);

        for(int h = 0; h <= slices; h++){
            verts.append(QVector3D(cos(h * hstep) * r, sin(h * hstep) * r, z));

            uv.append(QVector2D(float(h) / float(slices), 1.0f - float(v) / float(stacks)));

            if(h != slices && v != stacks){
                int w = slices + 1;
                triangles.append(((v    ) * w) + (h    ));
                triangles.append(((v + 1) * w) + (h    ));
                triangles.append(((v    ) * w) + (h + 1));

                triangles.append(((v + 1) * w) + (h + 1));
                triangles.append(((v    ) * w) + (h + 1));
                triangles.append(((v + 1) * w) + (h    ));


                lines.append(((v    ) * w) + (h    ));
                lines.append(((v + 1) * w) + (h    ));

                lines.append(((v    ) * w) + (h    ));
                lines.append(((v    ) * w) + (h + 1));
            }
        }
    }
}
