function update(entityId, dt)
    local camera = get_camera_component()
    if not camera then
        return
    end

    local transform = get_transform_component(entityId)
    if not transform then
        return
    end

    local sensitivity = 0.01
    local speed = 100.0
    local moveForward, moveRight = 0, 0

    -- e s d f movement instead of wasd, backspace and delete for up and down, right click to rotate
    if input:isKeyDown(GLFW_KEY_E) then
        moveForward = moveForward + 1
    end
    if input:isKeyDown(GLFW_KEY_D) then
        moveForward = moveForward - 1
    end
    if input:isKeyDown(GLFW_KEY_F) then
        moveRight = moveRight + 1
    end
    if input:isKeyDown(GLFW_KEY_S) then
        moveRight = moveRight - 1
    end
    if input:isKeyDown(GLFW_KEY_BACKSPACE) then
        transform:setPosition(transform:getPositionX(), transform:getPositionY() + speed * dt * 0.4, transform:getPositionZ())
    end
    if input:isKeyDown(GLFW_KEY_DELETE) then
        transform:setPosition(transform:getPositionX(), transform:getPositionY() - speed * dt * 0.4, transform:getPositionZ())
    end

    if moveForward ~= 0 or moveRight ~= 0 then
        local yaw = transform:getRotationY()
        local sinYaw = math.sin(yaw)
        local cosYaw = math.cos(yaw)

        local forwardX = sinYaw
        local forwardZ = -cosYaw

        local rightX = cosYaw
        local rightZ = sinYaw

        local dx = (forwardX * moveForward + rightX * moveRight) * speed * dt
        local dz = (forwardZ * moveForward + rightZ * moveRight) * speed * dt

        local x = transform:getPositionX() + dx
        local y = transform:getPositionY()
        local z = transform:getPositionZ() + dz
        transform:setPosition(x, y, z)
    end

    -- hold right click to rotate
    if input:isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) then

        local dx = input:getMouseDeltaX()
        local dy = input:getMouseDeltaY()
        local x = transform:getRotationX()
        local y = transform:getRotationY()

        local x = x - -dy * sensitivity
        local y = y - -dx * sensitivity

        -- prevent camera from flipping over
        x = math.max(-math.pi / 2, math.min(math.pi / 2, x))

        transform:setRotation(x, y, transform:getRotationZ())
    end
end

-- for mwheel up and down for zooming in and out
subscribe_mouse_scroll_event(function(x, y)
    local camera = get_camera_component()
    if not camera then return end
    camera:setFov(camera:getFov() - y)
end)
