function update(entityId, dt)
    local camera = get_camera_component()
    local projectionMatrix = camera:getProjectionMatrix()

    print("Projection Matrix:" .. camera:getFov())
end
