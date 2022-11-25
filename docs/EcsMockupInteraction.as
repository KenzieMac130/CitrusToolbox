class FPSPlayerComponent : ctKinnowComponent {
    float speed;
}

class FPSPlayerTickJob : ctKinnowJob {
    void Query(ctKinnowQueryInfo@ query){
        query.Require("FpsPlayer");
        query.Require("Location");
        query.Require("Rotation");
        query.Require("Scale");
    }

    void Execute(FPSPlayerComponent@ ctx, FPSPlayerComponent@ fps, ctKinnowLocation@ loc, ctKinnowRotation@ rot, ctKinnowScale@ scale){

        loc.y += fps.speed * ctx.deltaTime;
    }
}

class FPSPlayer : ctKinnowConcept {
    void Register(ctKinnowRegistry@ registry){
        registry.RegisterComponent("FpsPlayer", FPSPlayerComponent());
        registry.RegisterJob("PlayerTick", FPSPlayerTickJob());
    }
    
    void Spawn(ctKinnowSpawnContext@ ctx){
        ctx.AddComponent("Location");
        ctx.AddComponent("Rotation");
        ctx.AddComponent("Scale");
        ctx.AddComponent("FpsPlayer");
        ctx.AddComponent("Camera");
    }
}