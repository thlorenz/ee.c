DOCKER_HOST?=tcp://127.0.0.1:4243
IMAGE_NAME=thlorenz/ee_make
	
# substitution strips leading d, so that 'make dgrind' executes 'make grind' inside docker container
d%: clean
	echo 'from thlorenz/valgrind' > Dockerfile
	docker -H $(DOCKER_HOST) build -rm -t="$(IMAGE_NAME)" . 
	@rm -f Dockerfile

	docker -H $(DOCKER_HOST) run -i -rm $(IMAGE_NAME) $(@:d%=%)
