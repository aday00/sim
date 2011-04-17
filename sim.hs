

gravity = 9.8
dt = 0.01

-- Particle: position velocity
data Ptcl = Ptcl {
    mass     :: Float
  , position :: Float
  , velocity :: Float
} deriving (Show)

-- return new particle effected by gravity
-- f = ma => f / m = a => f/m = dv/dt => (f*dt)/m = dv
gravitate :: Ptcl -> Ptcl
gravitate p = p' where  
  dv  = (gravity * dt) / (mass p)
  vel = (velocity p) + dv
  p'  = Ptcl {
      mass     = mass p
    , position = (position p) - (vel * dt)
    , velocity = vel
  } 

main = do
  let p = Ptcl 1 10 0
  let p' = gravitate p
  putStrLn $ show p
  putStrLn $ show p'
