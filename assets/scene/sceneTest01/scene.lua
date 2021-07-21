function onZombieDie(signalInfo, eventDispatcher)
	print("Zombie was killed")
	eventDispatcher.Trigger("levelFolder/door", "open")
end

function defineMap(commandBuffer)
	commandBuffer.Begin()
	commandBuffer.CmdOpenSection("background")
	commandBuffer.CmdSetString("cube", "../env/TestCube.ktx") -- Set background cubemap
	commandBuffer.CmdSetVector("tint", 1,1,1) -- Set background tint
	commandBuffer.CmdSetScalar("strength", 1.0) -- Set background strength
	
	commandBuffer.CmdOpenSection("sun")
	commandBuffer.CmdSetVector("dir", 0,-1,0) -- Set sun direction
	commandBuffer.CmdSetScalar("strength", 0.5) -- Sun half strength
	commandBuffer.CmdSetVector("tint", 1,0,0) -- Sun color red
	
	commandBuffer.CmdOpenSection("game")
	commandBuffer.CmdSetString("type", "FPS")
	commandBuffer.CmdSetBool("pistol", true)
	commandBuffer.CmdSetScalar("health", 50)
	commandBuffer.CmdSetString("objective", "Get rid of zombies")
	commandBuffer.CmdSetVector("playerStartPos", 0,0,0)
	commandBuffer.CmdSetVector("playerStartRot",0,0,35)
	
	commandBuffer.CmdSpawnStageGLTF("mainLevel.gltf", "levelFolder", 0,0,0, 0,0,0) -- spawn an entire stage from GLTF (contains set dressing and spawners)
	commandBuffer.CmdSpawnToy("fps/enemy/zombie", "enemies/zombie/0", -1,0,15, 0,0,-35, "-health 45") -- not ideal, but possible
	commandBuffer.CmdSpawnToy("fps/enemy/zombie", "enemies/zombie/1", 0,0,15, 0,0,-35, "-health 45") -- not ideal, but possible
	commandBuffer.CmdSpawnToy("fps/enemy/zombie", "enemies/zombie/2", 1,0,15, 0,0,-35, "-health 45") -- not ideal, but possible
	
	commandBuffer.CmdLoadBarrier() -- wait for things to load (needed for prunes and sockets)
	
	commandBuffer.CmdSpawnToyRelative("common/particle", "levelFolder/fireHydrant", 0,0,0, 0,0,0, "-type fountain") -- Spawn relative to socket
	commandBuffer.CmdPruneToy("levelFolder/endGameContentItem") -- Removes a toy by path
	commandBuffer.CmdPruneTag("HARDMODE") -- Removes all toys of tags
	
	-- todo: get array of all sockets in folder
	--commandBuffer.Abort() -- Abort loading (will return failure on attempting to load level)
	commandBuffer.Finalize() -- Finalize the command loading properly (notifies to kick off loading set dressing/meshes/textures/etc)
end

function setupToys(toys, binder)
	binder.BindSignal("enemies/zombie/2", "onDeath", "onZombieDie")
end