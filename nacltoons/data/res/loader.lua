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
}

local PTM_RATIO = 32

local function ScreenToWorld(value)
    return value / PTM_RATIO
end

local function Log(...)
    print('LUA: '..string.format(...))
end

local function LoadGame(filename)
    Log('loading game: '..filename)
    local game = dofile(filename)
    Log('loading backgound image: '..game.background_image)
    Log('loading '..table.getn(game.levels)..' level(s)')
    game.filename = filename
    return game
end

local function AddShapeToBody(body, shape)
    Log('adding fixture')
    local shape_def = b2FixtureDef:new_local()
    shape_def.shape = shape
    shape_def.density = 1.0
    shape_def.friction = 0.5
    shape_def.restitution = 0.3
    Log('adding fixture 2')
    body:CreateFixture(shape_def)
    Log('adding fixture done')
end

local function AddSphereToBody(body, radius)
    local shape = b2CircleShape:new_local()
    shape.m_radius = ScreenToWorld(radius)
    AddShapeToBody(body, shape)
end

local function LoadLevel(game, level_number)
    Log('loading level '..level_number..' from '..game.filename)
    -- Get level descrition object
    local level = game.levels[level_number]

    -- Find the layer into which we want to load objects.
    local origin = CCDirector:sharedDirector():getVisibleOrigin()
    -- Log('finding layer: '..tags.LAYER_PHYSICS)
    -- local scene = CCDirector:sharedDirector():getRunningScene()
    -- local layer = scene:getChildByTag(tags.LAYER_PHYSICS)
    local layer = PhysicsLayer:GetCurrent()
    local world = layer:GetWorld()

    -- Create ball, a physics object
    local ball = CCPhysicsSprite:create(game.ball_image)
    layer:addChild(ball, 1, tags.BALL)
    local body_def = b2BodyDef:new_local()
    body_def.type = b2_dynamicBody
    local body = world:CreateBody(body_def)
    ball:setB2Body(body)
    ball:setPTMRatio(PTM_RATIO)
    ball:setPosition(origin.x + level.start[1], origin.y + level.start[2])
    AddSphereToBody(body, ball:boundingBox().size.height/2)

    -- Create goal and stars
    Log('goal: '..level.goal[1]..'x'..level.goal[2])
    local goal = CCSprite:create(game.goal_image)
    goal:setPosition(origin.x + level.goal[1], origin.y + level.goal[2])
    layer:addChild(goal, 1, tags.GOAL)

    game.star_image = CCTextureCache:sharedTextureCache():addImage(game.star_image)
    for i, star in ipairs(level.stars) do
        local sprite = CCSprite:createWithTexture(game.star_image)
        local tag = tags.STAR1 + i - 1
        Log('loading star [tag='..tag..']: '..star[1]..'x'..star[2])
        layer:addChild(sprite, 1, tag)
        sprite:setPosition(origin.x + star[1], origin.y + star[2])
    end

    game.background = CCTextureCache:sharedTextureCache():addImage(game.background_image)
end

game = LoadGame('sample_game/game.lua')
LoadLevel(game, 1)
