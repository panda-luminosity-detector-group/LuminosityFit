# Docker Image for Luminosity Fit

## Building

Depends on the PandaRoot docker image (in which KoalaSoft can be compiled and run as well).

Build with:

```
docker build -t localhost:5000/lumifit .
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
