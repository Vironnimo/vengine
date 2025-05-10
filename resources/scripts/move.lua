function update(entityId, dt)
    local speed = -5.0
    set_velocity(entityId, speed * dt, 0.0, 0.0)
end

-- print("move.lua loaded")
