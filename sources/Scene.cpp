#include "Scene.h"
#include <omp.h>

Scene::Scene(){
    debug = false;
    maxDepth = 5;
    outfile = "test.png";
    attenuation = Arr3(1, 0, 0);
}

void Scene::setFile(string file){
    outfile = file;
}

void Scene::setImage(istream& in){
    in >> imageW >> imageH;
    for (int i=0; i<imageH; i++){
        vector<Arr3> row;
        for (int j=0; j<imageW; j++)
            row.push_back(Arr3(0, 0, 0));
        image.push_back(row);
    }
    cout<<"set image "<<imageW<<" "<<imageH<<endl;
}

void Scene::setCamera(istream& in){
    cam = Camera(in);
    pixelSize = cam.calculatePixelSize(imageW, imageH);
    screenTopLeft = cam.at 
        - pixelSize.pixelW * imageW / 2
        - pixelSize.pixelH * imageH / 2;
}

void Scene::addLight(istream& in, bool dir){
    lights.push_back(Light(in, dir, attenuation));
}

Intersection Scene::firstObjHit(Ray ray){
    float t = 0;
    Obj *obj = nullptr;
    Arr3 point, normal;
    for (int i=0; i<objs.size(); i++){
        Arr3 pp, nn;

        if (!objs[i]->touchBox(ray)){
            continue;
        }

        if (!objs[i]->intersectWithRay(ray, pp, nn)){
            continue;
        }

        float tt = ray.getT(pp);

        if (tt > TOLERANCE && (t == 0 || tt < t)){
            t = tt;
            obj = objs[i];
            point = pp;
            normal = nn;
        }
    }
    
    return Intersection(obj, point, normal);
}

Arr3 Scene::shootRay(Ray ray, int depth){
    
    Arr3 res;
    if (depth == maxDepth) 
        return res;
    
    Intersection hit = firstObjHit(ray);

    if (hit.obj != nullptr) {

        // material color
        res.add(hit.obj->shadingVars.ambience + 
            (hit.obj->shadingVars.emission));

        // light shading color
        Arr3 normal = hit.nor.normalize(),
            eyedir = (cam.eye - hit.pos).normalize();

        for (Light light : lights){
            Ray ltRay(hit.pos, light.position, !light.directional);
            Intersection hit2 = firstObjHit(ltRay);
            bool toShade =
                hit2.obj == nullptr || 
                (!light.directional && 
                    (hit2.pos-(hit.pos)).length() > 
                    (light.position-(hit.pos)).length());
            if (toShade){
                res.add(light.shade(hit, eyedir, normal));
            }
        }

        // reflection color
        Arr3 destroyCPU = hit.obj->shadingVars.specular;
        if (destroyCPU.length() > 0) {
            Arr3 from = (hit.pos - cam.eye).normalize();
            Ray rfRay(hit.pos, from - (normal * from.dot(normal) * 2), false);
            res.add(shootRay(rfRay, depth+1) * destroyCPU);
        }
    }

    return res;
}

// TODO: change how loop through arrays work

void Scene::render(){
    
    long count = 0, counttill = 1000;
    
    #pragma omp parallel for

    // check pixel 8, 25 for scene 44 size /10.
    for (long k=0; k<imageW*imageH; k++){
        int i = k / imageW;
        int j = k % imageW;

        Ray ray(cam.eye, screenTopLeft + (pixelSize.pixelH*(i)) + 
            (pixelSize.pixelW*(j)) + (pixelSize.pixelMid), true);

        image[i][j] = shootRay(ray, 1).between();

        // progress counter
        #pragma omp critical
        if (++count % counttill == 0)
            cout<<"    Rendering "<<outfile<<": "<<(float)count/imageW*100/imageH<<"%  \r"<<flush;
        
    }

    cout<<"Rendering "<<outfile<<" complete             \n";
}