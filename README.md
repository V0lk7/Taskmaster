# Taskmaster

## Start Docker container

To start this project on a computer without needing to install anything, start the Docker container:

```sh
cd infra-dev
docker compose up -d
docker compose exec -it taskmaster zsh
```

This container is a basic Ubuntu system with the necessary project packages and a bind-mounted folder for accessing project files.

You can run the following instructions in the container.

## Compile the project using CMAKE

```sh
cmake -S . -B build
cmake --build build --parallel 8 --target install
```

To run the daemon taskmasterd :

```sh
./build/bin/taskmasterd ./confs/conf.yaml
```

You can launch the client when the daemon is started.

```sh
./build/bin/taskmasterctl ./confs/conf.yaml
```

## Documentation

- [Nanomsg lib](https://nanomsg.org/index.html)
- [Yaml CPP](https://github.com/jbeder/yaml-cpp/wiki/How-To-Parse-A-Document-(Old-API))
