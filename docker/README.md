# Docker Image for Luminosity Fit

## Using an image from the Docker Hub

```
singularity build lmdfitNov22p1.sif docker://rklasen/fairroot:miniNov22p1
```

## Building Offline from Local Docker Image

Depends on the PandaRoot docker image (in which KoalaSoft can be compiled and run as well).

Go to the root directory of this repository.

Build the Docker image with:

```
docker build -t localhost:5000/lumifit -f docker/Dockerfile .
```

### Setup local Docker Registry

```bash
docker run -d -p 5000:5000 --restart=always --name registry registry:2
```

Push image to local registry

```bash
docker push localhost:5000/lumifit
```

## Convert to Singularity Image

```bash
SINGULARITY_NOHTTPS=true singularity build lumifit.sif docker/singularityRecipe.txt
```

## Run the Software inside the Singularity Container

You can now enter the container with:

```
module load tools/Singularity
singularity run lmdfit-mini.sif
```
