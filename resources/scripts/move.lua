function update(entityId, dt)
    local transform = get_transform_component(entityId)
    if transform then
        local currentX = transform:getPositionX()
        local currentY = transform:getPositionY()
        local currentZ = transform:getPositionZ()

        -- print("Current Position: " .. currentX .. ", " .. currentY .. ", " .. currentZ)

        transform:setPosition(currentX - 10.0 * dt, currentY, currentZ)
        transform:updateMatrix()

        -- print("New Position X: " .. transform:getPositionX() .. ", Y: " .. transform:getPositionY() .. ", Z: " .. transform:getPositionZ())
    else
        print("Entity " .. entityId .. " has no TransformComponent")
    end
end

-- print("move.lua loaded")
