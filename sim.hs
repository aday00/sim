-- import qualified Data.Vector as V

gravity = 9.8
dt = 0.01

type Ft = Float -- default floating-point type to use.  Could be Double.

-- Particle: position velocity
data Ptcl = Ptcl {
    mass     :: Ft
  , position :: Ft
  , velocity :: Ft
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

-- gravitate until hit the ground
fall :: [Ptcl] -> [Ptcl]
fall ptcls@(p:ps) | position p <= 0 = ptcls
fall ptcls@(p:ps) = fall (p':ptcls) where
  p' = gravitate p

main = do
  mapM_ (putStrLn . show) (fall [Ptcl 1 10 0])
