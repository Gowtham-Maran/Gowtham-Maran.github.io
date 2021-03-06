
#include <ctime> 
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>

#include "test_scene.h"
#include "obj_loader.h"
#include "material.h"

#define RESOLUTION 1
#define SAMPLES 200

#define checkCudaErrors(val) check_cuda((val), #val, __FILE__, __LINE__)


void check_cuda(cudaError_t result, 
                char const *const func, 
                const char *const file, 
                int const line) {
    if(result) {
        std::cerr << "CUDA error = "<< static_cast<unsigned int>(result) << " at " <<
        file << ":" << line << " '" << func << "' \n";
        cudaDeviceReset();
        exit(99);
    }
}

__device__ vec3 shade(const Ray& r, 
                      Hitable **world, 
                      int depth,
                      curandState *state) {
    HitRecord rec;
    if((*world)->hit(r, 0.001, MAXFLOAT, rec)) {
        Ray scattered;
        vec3 attenuation;
        vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        if(depth < 15 && rec.mat_ptr->scatter(r, rec, attenuation, scattered, state)){
            return emitted + attenuation * shade(scattered, world, depth + 1, state);
        }else{
            return emitted;
        }
    }else{
        return vec3(0, 0, 0);
    }
}

/*
__device__ vec3 shade(const Ray& r, 
                      Hitable **world, 
                      int depth,
                      curandState *state) {
    Ray cur_ray = r;
    vec3 cur_attenuation = vec3(0.0,0.0,0.0);
    for(int i = 0; i < 5; i++) {
    	HitRecord rec;
    	if((*world)->hit(cur_ray, 0.001, MAXFLOAT, rec)) {
        	Ray scattered;
        	vec3 attenuation;
        	vec3 emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
        	if(rec.mat_ptr->scatter(cur_ray, rec, attenuation, scattered, state)){
            		cur_attenuation = emitted + cur_attenuation * attenuation;
			cur_ray = scattered;
        	}
		else{
            		return emitted;
        	}
    	}
	else{
        	return cur_attenuation;
    	}
   }
   return cur_attenuation; // exceeded recursion
}
*/

__device__ vec3 shade_nolight(const Ray& r, 
                              Hitable **world, 
                              int depth,
                              curandState *state) {
    HitRecord rec;
    if((*world)->hit(r, 0.001, MAXFLOAT, rec)) {
        Ray scattered;
        vec3 attenuation;
        if(depth < 15 && rec.mat_ptr->scatter(r, rec, attenuation, scattered, state)) {
            return attenuation * shade_nolight(scattered, world, depth + 1, state);
        }
        else {
            return vec3(0, 0, 0);
        }
    }else{
        return vec3(1.0, 1.0, 1.0);
    }
}

/*
__device__ vec3 shade_nolight(const Ray& r, 
                              Hitable **world, 
                              int depth,
                              curandState *state) {
    Ray cur_ray = r;
    vec3 cur_attenuation = vec3(0.0,0.0,0.0);
    for(int i = 0; i < 15; i++) {
    	HitRecord rec;
    	if((*world)->hit(cur_ray, 0.001, MAXFLOAT, rec)) {
        	Ray scattered;
        	vec3 attenuation;
        	if(rec.mat_ptr->scatter(cur_ray, rec, attenuation, scattered, state)) {
            		cur_attenuation *= attenuation;
            		cur_ray = scattered;
        	}
        	else {
            		return vec3(0, 0, 0);
        	}
    	}
    	else{
		return cur_attenuation;
    	}
   }
   return cur_attenuation; // exceeded recursion	
}
*/

__global__ void build_scene(Hitable** obj_list, 
                            Hitable** world,
                            Camera** camera,
                            curandState *state,
                            int nx,
                            int ny,
                            int cnt){
    if(threadIdx.x == 0 && blockIdx.x == 0) {

	
	//random_scene(obj_list, world, state);	
	//simple_light_scene(obj_list, world);
	//cornell_box_scene(obj_list, world);
	cornell_box_scene(obj_list, world);
	//cornell_smoke_scene(obj_list, world, state);
	//bvh_scene(obj_list, world, state);
	//final_scene(obj_list, world, state);
	
	//draw_some_triangles(obj_list, world, state);
	//draw_one_triangle(obj_list, world, state);

	
	
	/*
        vec3 lookfrom(0, 0, 20);
        vec3 lookat(0, 0, 0);
        float dist_to_focus = 10.0;
        float aperture = 0.0;
        float vfov = 60.0;

        *camera = new MotionCamera(lookfrom, 
                                lookat, 
                                vec3(0, 1, 0), 
                                vfov, 
                                float(nx) / float(ny), 
                                aperture, 
                                dist_to_focus,
                                0.0,
                                1.0);
	*/

	/*
	vec3 lookfrom(-500, 0, 500);
        vec3 lookat(-500, 0, -500);
        float dist_to_focus = 10.0;
        float aperture = 0.0;
        float vfov = 40.0;

        *camera = new MotionCamera(lookfrom, 
                                lookat, 
                                vec3(0, 1, 0), 
                                vfov, 
                                float(nx) / float(ny), 
                                aperture, 
                                dist_to_focus,
                                0.0,
                                1.0);
	*/

        
	vec3 lookfrom(278, 278, -800);
        vec3 lookat(278, 278, 0);
        float dist_to_focus = 10.0;
        float aperture = 0.0;
        float vfov = 40.0;

        *camera = new MotionCamera(lookfrom, 
                                lookat, 
                                vec3(0, 1, 0), 
                                vfov, 
                                float(nx) / float(ny), 
                                aperture, 
                                dist_to_focus,
                                0.0,
                                1.0);
        
	/*
        vec3 lookfrom(13,2,3);
        vec3 lookat(0,0,0);
        float dist_to_focus = 10.0; //(lookfrom-lookat).length();
        float aperture = 0.1;
        *camera   = new MotionCamera(lookfrom,
                                 lookat,
                                 vec3(0,1,0),
                                 30.0,
                                 float(nx)/float(ny),
                                 aperture,
                                 dist_to_focus,
				 0.0,
				 1.0);
	*/
    }
}

__global__ void build_mesh(Hitable** mesh,
                            Camera** camera,
                            Hitable** triangles,
                            vec3* points,
                            vec3* idxVertex,
                            int np, int nt,
                            curandState *state,
                            int nx, int ny, int cnt){
    if(threadIdx.x == 0 && blockIdx.x == 0) {

        //draw_one_mesh(mesh, triangles, points, idxVertex, np, nt, state);
        //bunny_inside_cornell_box(mesh, triangles, points, idxVertex, np, nt, state);
        
/*
	vec3 lookfrom(0, 0, 10);
        vec3 lookat(0, 0, 0);
        float dist_to_focus = 10.0;
        float aperture = 0.0;
        float vfov = 60.0;

        *camera = new MotionCamera(lookfrom, 
                                    lookat, 
                                    vec3(0, 1, 0), 
                                    vfov, 
                                    float(nx) / float(ny), 
                                    aperture, 
                                    dist_to_focus,
                                    0.0,
                                    1.0);
*/

	vec3 lookfrom(278, 278, -800);
        vec3 lookat(278, 278, 0);
        float dist_to_focus = 10.0;
        float aperture = 0.0;
        float vfov = 40.0;

        *camera = new MotionCamera(lookfrom, 
                                lookat, 
                                vec3(0, 1, 0), 
                                vfov, 
                                float(nx) / float(ny), 
                                aperture, 
                                dist_to_focus,
                                0.0,
                                1.0);

    }
}

__global__ void random_init(int nx, 
                            int ny, 
                            curandState *state) {
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    if((x >= nx) || (y >= ny)) return;
    int pixel_index = y * nx + x;
    curand_init(0, pixel_index, 0, &state[pixel_index]);
}


__global__ void destroy(Hitable** obj_list, 
                        Hitable** world,
                        Camera** camera, 
                        int obj_cnt) {
    for(int i = 0; i < obj_cnt; i++) {
        delete *(obj_list + i);
    }
    delete *world;
    delete *camera;
}


__global__ void render(vec3* colorBuffer,
                       Hitable** world,
                       Camera** camera,
                       curandState* state,
                       int nx, 
                       int ny,
                       int samples) {
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;
    if((x >= nx) || (y >= ny)) return;
    
    int pixel_index = y * nx + x;

    int ns = samples;
    vec3 col(0, 0, 0);
    for(int i = 0; i < ns; i++){
        float u = float(x + rand(&(state[pixel_index]))) / float(nx);
        float v = float(y + rand(&(state[pixel_index]))) / float(ny);
        Ray r = (*camera)->get_ray(u, v, state);
        col += shade(r, world, 0, &(state[pixel_index]));
        //col += shade_nolight(r, world, 0, &(state[pixel_index]));
    }
    col /= float(ns);
    col[0] = sqrt(col[0]);
    col[1] = sqrt(col[1]);
    col[2] = sqrt(col[2]);

    colorBuffer[pixel_index] = clip(col);
}


int main() {
    std::time_t tic = std::time(NULL);
    std::cout << "Start running at: " << std::asctime(std::localtime(&tic)) << std::endl;

    std::ofstream imgWrite("./images/image.ppm");

    //int nx = 1024 * RESOLUTION;
    //int ny = 512  * RESOLUTION;
    int nx = 1280 * RESOLUTION;
    int ny = 720  * RESOLUTION;
    //int tx = 16;
    //int ty = 16;
    int tx = 8;
    int ty = 8;

    int num_pixel = nx * ny;

    // color buffer
    vec3 *colorBuffer;
    checkCudaErrors(cudaMallocManaged((void**)& colorBuffer, num_pixel*sizeof(vec3)));

    curandState* curand_state;
    checkCudaErrors(cudaMallocManaged((void**)& curand_state, num_pixel*sizeof(curandState)));

    // build world
    int obj_cnt = 488;
    Hitable** obj_list;
    Hitable** world;
    Camera**  camera;
    checkCudaErrors(cudaMallocManaged((void**)& obj_list, obj_cnt*sizeof(Hitable*)));
    checkCudaErrors(cudaMallocManaged((void**)& world, sizeof(Hitable*)));
    checkCudaErrors(cudaMallocManaged((void**)& camera, sizeof(Camera*)));

    dim3 blocks(nx / tx + 1, ny / ty + 1);
    dim3 threads(tx, ty);
    random_init <<<blocks, threads>>>(nx, ny, curand_state);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    /*
    // --------------------------- allocate the mesh ----------------------------------------
    vec3* points;
    vec3* idxVertex;

    // NOTE: must pre-allocate before initialize the elements

    checkCudaErrors(cudaMallocManaged((void**)& points,    2600 * sizeof(vec3)));
    checkCudaErrors(cudaMallocManaged((void**)& idxVertex, 5000 * sizeof(vec3)));

    int nPoints, nTriangles;
    parseObjByName("./shapes/small_bunny.obj", points, idxVertex, nPoints, nTriangles);
    std::cout << "# of points: " << nPoints << std::endl;
    std::cout << "# of triangles: " << nTriangles << std::endl;

    // scale

    for(int i = 0; i < nPoints; i++) { points[i] *= 30.0; }
    for(int i = 0; i < nPoints; i++) { std::cout << points[i] << std::endl; }

    Hitable** triangles;
    checkCudaErrors(cudaMallocManaged((void**)& triangles, nTriangles * sizeof(Hitable*)));

    // --------------------------- ! allocate the mesh ---------------------------------------

    build_mesh <<<1, 1>>>(world, camera, triangles, points,idxVertex, nPoints, nTriangles, curand_state, nx, ny, obj_cnt);
    */

    build_scene <<<1, 1>>>(obj_list, world, camera, curand_state, nx, ny, obj_cnt);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    render <<<blocks, threads>>>(colorBuffer, world, camera, curand_state, nx, ny, SAMPLES);
    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaDeviceSynchronize());

    imgWrite << "P3\n" << nx << " " << ny << "\n255\n";
    for(int i = ny - 1; i >= 0; i--) {
        for(int j = 0; j < nx; j++) {
            size_t pixel_index = i * nx + j;
            int ir = int(255.99 * colorBuffer[pixel_index].r());
            int ig = int(255.99 * colorBuffer[pixel_index].g());
            int ib = int(255.99 * colorBuffer[pixel_index].b());
            imgWrite << ir << " " << ig << " " << ib << "\n";
        }
    }
    // clean up
    //checkCudaErrors(cudaDeviceSynchronize());
    destroy<<<1, 1>>>(obj_list, world, camera, obj_cnt);

    checkCudaErrors(cudaGetLastError());
    checkCudaErrors(cudaFree(world));
    checkCudaErrors(cudaFree(obj_list));
    checkCudaErrors(cudaFree(camera));
    checkCudaErrors(cudaFree(curand_state));
    checkCudaErrors(cudaFree(colorBuffer));

    cudaDeviceReset();

    std::time_t toc = std::time(NULL);
    std::cout << "Finish running at: " << std::asctime(std::localtime(&toc)) << std::endl;
    std::cout << "Time consuming: " << toc - tic << "s" << std::endl;
}

