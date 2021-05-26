math.randomseed(os.time())

newCharacter = Character:NewCharacter();

newCharacter:SetLocation(
    2500 + math.random() * -5000,
    2500 + math.random() * -5000,
    0
)

newCharacter:SetRotation(
    0,
    math.random() * -5000,
    0
)