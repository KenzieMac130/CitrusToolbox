function loadScene(sceneIn)
	scene.spawnInternalToy(sceneIn, "fps/player", -50,0,0, math.rad(90),0,0, 1,"")
	scene.spawnInternalToy(sceneIn, "citrus/groundPlane", 0,-0.5,0, 0,0,0,1,"")
	
	for i = 0, 1024, 1
	do
		--str = string.format("%f %f %f 1", math.random(), math.random(), math.random())
		scene.spawnInternalToy(sceneIn, "citrus/testShape", 0,i,64, 0,0,0, 1, "1 0 0 1")
	end
	
	for i = 0, 32, 1
	do
		for j = 0, 32, 1
		do
			--str = string.format("%f %f %f 1", math.random(), math.random(), math.random())
			scene.spawnInternalToy(sceneIn, "citrus/testShape", j,i,32, 0,0,0, 1, "0 1 0 1")
		end
	end	
	
	for i = 0, 3, 1
	do
		for j = 0, 64, 1
		do
			for k = 0, 0, 1
			do
				--str = string.format("%f %f %f 1", math.random(), math.random(), math.random())
				scene.spawnInternalToy(sceneIn, "citrus/testShape", j,i,k, math.rad(0),math.rad(0),0, 1, "0 0 1 1")
			end
		end
	end
end