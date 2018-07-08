-- GameObjPtr newGameObj = Character::StaticCreate();
-- CharacterPtr newCharacter = std::static_pointer_cast< Character >( newGameObj );

-- newCharacter->SetPlayerId( cliProxy->GetPlayerId() );
-- newCharacter->SetLocation( Vector3(
--     2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
--     2500.f + RealtimeSrvMath::GetRandomFloat() * -5000.f,
--     0.f ) );
-- newCharacter->SetRotation( Vector3(
--     0.f,
--     RealtimeSrvMath::GetRandomFloat() * 180.f,
--     0.f ) );

-- return newGameObj;



-----------------------------

math.randomseed(os.time())

-- print(math.random())

newCharacter = Character.new();

-- newCharacter:SetPlayerId();

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