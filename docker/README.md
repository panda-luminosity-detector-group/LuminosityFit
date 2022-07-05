# Docker Image for Luminosity Fit

## Building

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
SINGULARITY_NOHTTPS=true singularity build lumifit.sif singularityRecipe.txt
```

## Run the Software inside the Singularity Container

You can now enter the container with:

```
module load tools/Singularity
singularity run lmdfit-mini.sif
```
