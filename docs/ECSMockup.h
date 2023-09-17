/* ECS proof of concept (design something pleasant to work with and non-dogmatic) */

class LocationComponent {
public:
    ctVec3 value;
};

class PlayerStateComponent {
public:
    float health;
};

class AnimationControllerComponent {
public:
    SetProp()

private:
    /* consider animation controller sandboxed */
    class ctAnimationController *pController;
};

/* todo component reflection using citrus reflect */

class PlayerKillZoneSystem : public ctSceneSystemParallelCached {
    virtual void DeclareDependencies(){
        ExternalDependency("Renderer"); /* example, not really here */
        ComponentDependency("Location", READ, 0);
        ComponentDependency("PlayerState", READ_WRITE, 1);
        ComponentDependency("AnimController", WRITE, 2);
    }

    virtual void ScheduleSetup(SetupContext ctx){
        globalKillPlane = ctx.Scene->GetKillPlane();
    }

    virtual void Execute(uint32_t index, float deltaTime) const {
        LocationComponent& location = GetComponent(0, index);
        PlayerStateComponent& player = GetComponent(1, index);
        AnimationControllerComponent& anim = GetComponent(2, index);
        if(location.z < globalKillPlane)
        player.health -= 10.0f * deltaTime;
        anim.SetProp("Hurt", 1.0f);
    }

    float globalKillPlane = 0.0f;
};

void OnStartup() {
    ctSceneComponentRegister(LocationComponent);
    ctSceneSystemRegister(PlayerKillZoneSystem);
}

void TickUpdate() {
    ctSceneSystemSchedule(PlayerKillZoneSystem);
};