
#include <curand.h>
#include <curand_kernel.h>

#include "float.h"

#include "bvh.h"
#include "transform.h"
#include "medium.h" 

#include "ray.h"
#include "vec3.h"
#include "mesh.h"
#include "box.h"
#include "camera.h"
#include "sphere.h"
#include "triangle.h"
#include "rectangle.h"
#include "material.h"
#include "hitable_list.h"   


#define RAND rand(state)


__device__ float rand(curandState *state){
    return float(curand_uniform(state));
}


/* It works */
__device__ void random_scene(Hitable **list, 
                             Hitable **world, 
                             curandState *state){
    Texture *checker = new CheckerTexture(new ConstantTexture(vec3(0.2, 0.3, 0.1)),
                                          new ConstantTexture(vec3(0.9, 0.9, 0.9)));
    list[0] = new Sphere(vec3(0, -1000.0, -1), 1000, new Lambertian(checker));
    int i = 1;
    for(int a = -11; a < 11; a++) {
        for(int b = -11; b < 11; b++) {
            float choose_mat = rand(state);
            vec3 center(a + 0.9 * rand(state), 0.2, b + 0.9 * rand(state));
            if(choose_mat < 0.8f) {
                list[i++] = new MovingSphere(center, center+vec3(0, 0.5*rand(state), 0), 0.0, 1.0, 0.2,
                                             new Lambertian(new ConstantTexture(vec3(rand(state), rand(state), rand(state)))));
                continue;
            }
            else if(choose_mat < 0.95f) {
                list[i++] = new Sphere(center, 0.2,
                                       new Metal(vec3(0.5f*(1.0f+rand(state)), 
                                                      0.5f*(1.0f+rand(state)), 
                                                      0.5f*(1.0f+rand(state))), 
                                                      0.5f*rand(state)));
            }
            else {
                list[i++] = new Sphere(center, 0.2, new Dielectric(rand(state)*2));
            }
        }
    }
    list[i++] = new Sphere(vec3( 0, 1, 0), 1.0, new Dielectric(1.5));
    list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(checker));
    list[i++] = new Sphere(vec3( 4, 1, 0), 1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));
    *world = new HitableList(list, i);
}


/* It works */
__device__ void simple_light_scene(Hitable **list, 
                                   Hitable **world){
    Texture *checker = new CheckerTexture(new ConstantTexture(vec3(0.8, 0.3, 0.1)),
                                          new ConstantTexture(vec3(0.9, 0.9, 0.9)));

    list[0] = new Sphere(vec3(0, -1000, 0), 1000, new Lambertian(checker));
    list[1] = new Sphere(vec3(0,     2, 0),    2, new Lambertian(checker));
    list[2] = new Sphere(vec3(0,     7, 0),    2, new DiffuseLight(new ConstantTexture(vec3(1, 0, 0))));
    list[3] = new RectangleXY(3, 5, 1, 3, -2,     new DiffuseLight(new ConstantTexture(vec3(4, 4, 4))));
    *world  = new HitableList(list, 4);
}

/* It works */
__device__ void cornell_box_scene(Hitable **list, 
                                  Hitable **world){
    int i = 0; 
    Material* red   = new   Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
    Material* white = new   Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
    Material* green = new   Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new ConstantTexture(vec3(  15,   15,   15)));

    Material* diele = new Dielectric(0.5);
    Material* metal = new Metal(vec3(0.7, 0.6, 0.5), 0.3);

    list[i++] = new FlipNormals(new RectangleYZ(  0, 555,   0, 555, 555, green));
    list[i++] =                (new RectangleYZ(  0, 555,   0, 555,   0,   red));
    list[i++] =                (new RectangleXZ(213, 343, 227, 332, 554, light));
    list[i++] = new FlipNormals(new RectangleXZ(  0, 555,   0, 555, 555, white));
    list[i++] =                (new RectangleXZ(  0, 555,   0, 555,   0, white));
    list[i++] = new FlipNormals(new RectangleXY(  0, 555,   0, 555, 555, white));

    list[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 165, 165), metal), -18), vec3(130, 0,  65));
    list[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 330, 165), diele),  18), vec3(265, 0, 295));
    
    *world = new HitableList(list, i);
}

//with bvh
__device__ void cornell_box_scene1(Hitable **list, 
                                  Hitable **world,
                                  curandState *state){
    int i = 0; 
    Material* red   = new   Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
    Material* white = new   Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
    Material* green = new   Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new ConstantTexture(vec3(  15,   15,   15)));

    Material* diele = new Dielectric(0.5);
    Material* metal = new Metal(vec3(0.7, 0.6, 0.5), 0.3);

    Hitable** boxlist1 = new Hitable*[8];

    boxlist1[i++] = new FlipNormals(new RectangleYZ(  0, 555,   0, 555, 555, green));
    boxlist1[i++] =                (new RectangleYZ(  0, 555,   0, 555,   0,   red));
    boxlist1[i++] =                (new RectangleXZ(213, 343, 227, 332, 554, light));
    boxlist1[i++] = new FlipNormals(new RectangleXZ(  0, 555,   0, 555, 555, white));
    boxlist1[i++] =                (new RectangleXZ(  0, 555,   0, 555,   0, white));
    boxlist1[i++] = new FlipNormals(new RectangleXY(  0, 555,   0, 555, 555, white));

    boxlist1[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 165, 165), metal), -18), vec3(130, 0,  65));
    boxlist1[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 330, 165), diele),  18), vec3(265, 0, 295));

    int l = 0;
    list[l++] = new BVHNode(boxlist1, i, 0, 1, state);
    
    *world = new HitableList(list, l);
}

/* It works */
__device__ void cornell_smoke_scene(Hitable **list, 
                                    Hitable **world, 
                                    curandState *state){
    int i = 0; 
    Material* red   = new   Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
    Material* white = new   Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
    Material* green = new   Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new ConstantTexture(vec3(  15,   15,   15)));

    list[i++] = new FlipNormals(new RectangleYZ(  0, 555,   0, 555, 555, green));
    list[i++] =                (new RectangleYZ(  0, 555,   0, 555,   0,   red));
    list[i++] =                (new RectangleXZ(213, 343, 227, 332, 554, light));
    list[i++] = new FlipNormals(new RectangleXZ(  0, 555,   0, 555, 555, white));
    list[i++] =                (new RectangleXZ(  0, 555,   0, 555,   0, white));
    list[i++] = new FlipNormals(new RectangleXY(  0, 555,   0, 555, 555, white));

    Hitable* b1 = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0,  65));
    Hitable* b2 = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 330, 165), white),  15), vec3(265, 0, 295));
    list[i++] = new ConstantMedium(b1, 0.01, new ConstantTexture(vec3(1.0, 1.0, 1.0)), state);
    list[i++] = new ConstantMedium(b2, 0.01, new ConstantTexture(vec3(0.0, 0.0, 0.0)), state);
    
    *world = new HitableList(list, i);
}


/* Test BVH */
__device__ void bvh_scene1(Hitable **list, 
                          Hitable **world, 
                          curandState *state){
    int nb = 10;
    Hitable** boxlist1 = new Hitable*[1000];
    Material* ground = new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53)));
    
    int b = 0;
    for(int i = 0; i < nb; i++) {
        for(int j = 0; j < nb; j++) {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100 * (rand(state) + 0.01);
            float z1 = z0 + w;
            boxlist1[b++] = new Box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
        }
    }

    int l = 0;
    list[l++] = new BVHNode(boxlist1, b, 0, 1, state);
    
    *world = new HitableList(list, l);
}

//without bvh
__device__ void bvh_scene(Hitable **list, 
                          Hitable **world, 
                          curandState *state){
    int nb = 10;
    Hitable** boxlist1 = new Hitable*[1000];
    Material* ground = new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53)));
    
    int b = 0;
    for(int i = 0; i < nb; i++) {
        for(int j = 0; j < nb; j++) {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100 * (rand(state) + 0.01);
            float z1 = z0 + w;
            list[b++] = new Box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
        }
    }
    
    *world = new HitableList(list, b);
}

/* It works eventually*/
__device__ void final_scene1(Hitable **list, 
                            Hitable **world, 
                            curandState *state) {
    int nb = 20;
    Hitable** boxlist1 = new Hitable*[1000];
    Hitable** boxlist2 = new Hitable*[1000];
    Material* white  = new Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
    Material* ground = new Lambertian(new ConstantTexture(vec3(0.48, 0.83, 0.53)));
    
    int b = 0;
    for(int i = 0; i < nb; i++) {
        for(int j = 0; j < nb; j++) {
            float w = 100;
            float x0 = -1000 + i*w;
            float z0 = -1000 + j*w;
            float y0 = 0;
            float x1 = x0 + w;
            float y1 = 100 * (rand(state) + 0.01);
            float z1 = z0 + w;
            boxlist1[b++] = new Box(vec3(x0, y0, z0), vec3(x1, y1, z1), ground);
        }
    }

    int l = 0;
    list[l++] = new BVHNode(boxlist1, b, 0, 1, state);
    Material* light = new DiffuseLight(new ConstantTexture(vec3(7, 7, 7)));
    list[l++] = new RectangleXZ(123, 423, 147, 412, 554, light);
    vec3 center(400, 400, 200);
    list[l++] = new MovingSphere(center, center+vec3(30, 0, 0), 0, 1, 50, new Lambertian(new ConstantTexture(vec3(0.7, 0.3, 0.1))));
    list[l++] = new Sphere(vec3(260, 150,  45), 50, new Dielectric(1.5));
    list[l++] = new Sphere(vec3(  0, 150, 145), 50, new Metal(vec3(0.8, 0.8, 0.9), 10.0));
    Hitable* boundary = new Sphere(vec3(360, 150, 145), 70, new Dielectric(1.5));
    list[l++] = boundary;
    list[l++] = new ConstantMedium(boundary, 0.2, new ConstantTexture(vec3(0.2, 0.4, 0.9)), state);

    int ns = 1000;
    for(int j = 0; j < ns; j++){
        boxlist2[j] = new Sphere(vec3(165 * rand(state), 165 * rand(state), 165 * rand(state)), 10, white);   
    }
    list[l++] = new Translate(new Rotate(new BVHNode(boxlist2, ns, 0.0, 1.0, state), 15), vec3(-100, 270, 395));
    
    *world = new HitableList(list, l);
}


/*
__device__ void final_scene1(Hitable **list, 
                            Hitable **world, 
                            curandState *state) {
{
        list[0] = new Sphere(vec3(0,-1000.0,-1), 1000,
                               new Lambertian(new ConstantTexture(vec3(0.5, 0.5, 0.5))));
        int i = 1;
        for(int a = -11; a < 11; a++) {
            for(int b = -11; b < 11; b++) {
                float choose_mat = rand(state);
                vec3 center(a+rand(state),0.2,b+rand(state));
                if(choose_mat < 0.8f) {
                    list[i++] = new Sphere(center, 0.2,
                                             new Lambertian(new ConstantTexture(vec3(rand(state)*rand(state), rand(state)*rand(state), rand(state)*rand(state)))));
                }
                else if(choose_mat < 0.95f) {
                    list[i++] = new Sphere(center, 0.2,
                                             new Metal(vec3(0.5f*(1.0f+rand(state)), 0.5f*(1.0f+rand(state)), 0.5f*(1.0f+rand(state))), 0.5f*rand(state)));
                }
                else {
                    list[i++] = new Sphere(center, 0.2, new Dielectric(1.5));
                }
            }
        }
        list[i++] = new Sphere(vec3(0, 1,0),  1.0, new Dielectric(1.5));
        list[i++] = new Sphere(vec3(-4, 1, 0), 1.0, new Lambertian(new ConstantTexture(vec3(0.4, 0.2, 0.1))));
        list[i++] = new Sphere(vec3(4, 1, 0),  1.0, new Metal(vec3(0.7, 0.6, 0.5), 0.0));
        *world  = new HitableList(list, 22*22+1+3);
}
*/

__device__ void draw_one_triangle(Hitable **list, 
                                  Hitable **world, 
                                  curandState *state) {
    int l = 0;
    vec3 vertices[3] = {vec3(0, 10, -10), vec3(10, -6, -10), vec3(-10, -6, -10)};
    Material* mat = new DiffuseLight(new ConstantTexture(vec3(0.4, 0.7, 0.5)));

    list[l++] = new Triangle(vertices, mat, false);

    *world = new HitableList(list, l);
}

__device__ void draw_some_triangles(Hitable **list, 
                                  Hitable **world, 
                                  curandState *state) {
    int l = 0;
    int k = 0, m = 0;
    int n = 400;
    int d = 40;
    for(int i=0; i<n; i++){
	if(i%2 == 0)
		k = i%d;
	else
		k = -i%d;
	if((i/d) % 2 == 0)
		m = i/d;
	else
		m = -i/d;

    	vec3 vertices[3] = {vec3(k, m, -10), vec3(k, m + 1, -10), vec3(k + 1, m, -10)};
    	Material* mat = new DiffuseLight(new ConstantTexture(vec3(0.4, 0.7, 0.5)));

    	list[l++] = new Triangle(vertices, mat, false);
    }
    *world = new HitableList(list, l);
}

__device__ void draw_one_mesh(Hitable** mesh, 
                              Hitable** triangles,
                              vec3* points,
                              vec3* idxVertex,
                              int np, int nt,
                              curandState *state) {

    Material* mat = new DiffuseLight(new ConstantTexture(vec3(0.4, 0.7, 0.5)));

    int l = 0;
    for (int i = 0; i < nt; i++) {
        vec3 idx = idxVertex[i];
        vec3 v[3] = {points[int(idx[2])], points[int(idx[1])], points[int(idx[0])]};
        triangles[l++] = new Triangle(v, mat, true);
    }
    *mesh = new BVHNode(triangles, l, 0, 1, state);
}


__device__ void bunny_inside_cornell_box(Hitable** world,
                                         Hitable** list,
                                         vec3* points,
                                         vec3* idxVertex,
                                         int np, int nt,
                                         curandState *state){
    int i = 0; 
    Material* red   = new   Lambertian(new ConstantTexture(vec3(0.65, 0.05, 0.05)));
    Material* white = new   Lambertian(new ConstantTexture(vec3(0.73, 0.73, 0.73)));
    Material* green = new   Lambertian(new ConstantTexture(vec3(0.12, 0.45, 0.15)));
    Material* light = new DiffuseLight(new ConstantTexture(vec3(  15,   15,   15)));

    Material* diele = new Dielectric(0.5);
    Material* metal = new Metal(vec3(0.7, 0.6, 0.5), 0.3);

    list[i++] = new FlipNormals(new RectangleYZ(  0, 555,   0, 555, 555, green));
    list[i++] =                (new RectangleYZ(  0, 555,   0, 555,   0,   red));
    list[i++] =                (new RectangleXZ(213, 343, 227, 332, 554, light));
    list[i++] = new FlipNormals(new RectangleXZ(  0, 555,   0, 555, 555, white));
    list[i++] =                (new RectangleXZ(  0, 555,   0, 555,   0, white));
    list[i++] = new FlipNormals(new RectangleXY(  0, 555,   0, 555, 555, white));

    list[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 165, 165), metal), -18), vec3(130, 0,  65));
    list[i++] = new Translate(new Rotate(new Box(vec3(0, 0, 0), vec3(165, 330, 165), diele),  18), vec3(265, 0, 295));
    

    Material* bunny = new Metal(vec3(0.4, 0.7, 0.5), 0.5f*rand(state));

    int l = 0;
    for (int i = 0; i < nt; i++) {
        vec3 idx = idxVertex[i];
        vec3 v[3] = {points[int(idx[2])], points[int(idx[1])], points[int(idx[0])]};
        list[l++] = new Triangle(v, bunny, true);
    }

    *world = new HitableList(list, i);
}
