-- Main entry point for use of lua in the game.
-- Currently when this file is run it will load the game data from
-- the 'game1' folder and populate the scene with sprites.

-- Tags used when lua creates nodes. This allows the C++ side to
-- look up the nodes using these tags.
local tags = {
    LAYER_PHYSICS = 100,
    LAYER_UI = 101,
    BALL = 1,
    GOAL = 2,
    STAR1 = 3,
    STAR2 = 4,
    STAR3 = 5,
    BRUSH = 6,
    OBJECTS_START = 256,
}

local PTM_RATIO = 32

-- Convert a value from screen coordinate system to Box2D world coordinates
local function ScreenToWorld(value)
    return value / PTM_RATIO
end

--- Log messages to console
local function Log(...)
    print('LUA: '..string.format(...))
end

--- Load game data structure from the given lua file
local function LoadGame(filename)
    Log('loading game: '..filename)
    local game = dofile(filename)
    Log('loading backgound image: '..game.background_image)
    Log('loading '..table.getn(game.levels)..' level(s)')
    game.filename = filename
    return game
end

local function AddShapeToBody(body, shape, sensor)
    Log('adding fixture')
    local fixture_def = b2FixtureDef:new_local()
    fixture_def.shape = shape
    fixture_def.density = 1.0
    fixture_def.friction = 0.5
    fixture_def.restitution = 0.3
    fixture_def.isSensor = sensor
    Log('adding fixture 2')
    body:CreateFixture(fixture_def)
    Log('adding fixture done')
end

local function AddSphereToBody(body, radius, sensor)
    local shape = b2CircleShape:new_local()
    shape.m_radius = ScreenToWorld(radius)
    AddShapeToBody(body, shape, sensor)
end

--- Create a fix pivot point between the world and the given object.
local function CreatePivot(world, anchor, body)
    Log('creating anchor: '..anchor.x..'x'..anchor.y)
    local anchor_point = b2Vec2:new_local(ScreenToWorld(anchor.x),
                                          ScreenToWorld(anchor.y))

    -- create a new fixed body to pivot against
    local ground_def = b2BodyDef:new_local()
    ground_def.position = anchor_point
    local ground_body = world:CreateBody(ground_def);

    -- create the pivot joint
    local joint_def = b2RevoluteJointDef:new_local()
    joint_def:Initialize(ground_body, body, anchor_point)
    local joint = world:CreateJoint(joint_def)
end

--- Create a line between two points in the given Box2D world.
-- The brush is used to create a sequence of sprites to represent
-- the line.
local function CreateLine(world, brush, from, to, object)
    -- create body
    local body_def = b2BodyDef:new_local()
    body_def.position:Set(ScreenToWorld(from.x), ScreenToWorld(from.y))
    if object.type == 'dynamic' then
        Log('creating dynamic object')
        body_def.type = b2_dynamicBody
    end
    local body = world:CreateBody(body_def)

    if object.anchor ~= nil then
        CreatePivot(world, object.anchor, body)
    end

    -- calculate thickness based on brush sprite size
    local brush_tex = brush:getTexture()
    local brush_size = brush_tex:getContentSizeInPixels()
    local thickness = math.max(brush_size.height/2, brush_size.width/2);
    Log('thickness: '..thickness)

    -- calculate length and angle of line based on start and end points
    local length = ccpDistance(from, to);
    local dist_x = to.x - from.x
    local dist_y = to.y - from.y
    local angle = math.atan2(dist_y, dist_x)
    Log('loading object at: '..from.x..'x'..from.y..' angle: '..angle)

    -- create fixture
    local center = b2Vec2:new_local(ScreenToWorld(dist_x/2),
                                    ScreenToWorld(dist_y/2))
    local shape = b2PolygonShape:new_local()
    shape:SetAsBox(ScreenToWorld(length/2), ScreenToWorld(thickness),
                   center, angle)
    local fixture_def = b2FixtureDef:new_local()
    fixture_def.shape = shape
    fixture_def.density = 1.0
    fixture_def.friction = 0.5
    fixture_def.restitution = 0.3
    body:CreateFixture(fixture_def)

    -- Now create a visible CCPhysicsSprite that the body is attached to
    local sprite = CCPhysicsSprite:createWithTexture(brush_tex)
    sprite:setB2Body(body)
    sprite:setPTMRatio(PTM_RATIO)

    -- And add a sequence of non-physics sprites as children of the first
    local dist = CCPointMake(dist_x, dist_y)
    local num_children = length / thickness
    local inc_x = dist_x / num_children
    local inc_y = dist_y / num_children
    local child_location = CCPointMake(thickness, thickness)
    for i = 1,num_children do
        child_location.x = child_location.x + inc_x
        child_location.y = child_location.y + inc_y

        local child_sprite = CCSprite:createWithTexture(brush_tex)
        child_sprite:setPosition(child_location)
        sprite:addChild(child_sprite)
    end

    return sprite
end

--- Create CCPoint from a lua table containing 2 elements.
-- The point is then offset according the origin.
local function PointFromLua(origin, point)
    return CCPointMake(point[1] + origin.x, point[2] + origin.y)
end

--- Create a physics sprite at a fiven location with a given image
local function CreatePhysicsSprite(b2d_world, location, image, sensor)
    Log('goal: '..location.x..'x'..location.y)
    local ball = CCPhysicsSprite:create(image)
    local body_def = b2BodyDef:new_local()
    if not sensor then
        body_def.type = b2_dynamicBody
    end
    local body = b2d_world:CreateBody(body_def)
    ball:setB2Body(body)
    ball:setPTMRatio(PTM_RATIO)
    ball:setPosition(location)
    AddSphereToBody(body, ball:boundingBox().size.height/2, sensor)
    return ball
end

--- Load the given level of the given game
-- @param game lua dictionary containing game data
-- @param level_number the level to load
local function LoadLevel(game, level_number)
    Log('loading level '..level_number..' from '..game.filename)
    -- Get level descrition object
    local level = game.levels[level_number]

    -- Find the layer into which we want to load objects.
    local origin = CCDirector:sharedDirector():getVisibleOrigin()
    local layer = PhysicsLayer:GetCurrent()
    local world = layer:GetWorld()

    local ball = CreatePhysicsSprite(world, PointFromLua(origin, level.start),
                                     game.ball_image, false)
    layer:addChild(ball, 1, tags.BALL)

    -- Load brush image
    local brush = CCSpriteBatchNode:create(game.brush_image)
    layer:addChild(brush, 1, tags.BRUSH)

    -- Create goal and stars
    Log('goal: '..level.goal[1]..'x'..level.goal[2])
    local goal = CreatePhysicsSprite(world, PointFromLua(origin, level.goal),
                                     game.goal_image, true)
    layer:addChild(goal, 1, tags.GOAL)

    for i, star in ipairs(level.stars) do
        local sprite = CreatePhysicsSprite(world, PointFromLua(origin, star),
                                           game.star_image, true)
        local tag = tags.STAR1 + i - 1
        Log('loading star [tag='..tag..']: '..star[1]..'x'..star[2])
        layer:addChild(sprite, 1, tag)
    end

    -- Load fixtures
    for i, object in ipairs(level.objects) do
        local start = PointFromLua(origin, object.start)
        local finish = PointFromLua(origin, object.finish)
        if object.anchor ~= nil then
            object.anchor = PointFromLua(origin, object.anchor)
        end
        local line = CreateLine(world, brush, start, finish, object)
        brush:addChild(line, 1, tags.OBJECTS_START+i-1)
    end
end

-- Global variables that can be used in the lua data files.
height = 600
width = 800

local game = LoadGame('sample_game/game.lua')
LoadLevel(game, 1)
