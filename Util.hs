module Util where

--type Ft = Float -- default floating-point type to use.  Could be Double.
type Ft = Double -- default floating-point type to use.  Could be Double.

deltatime = 0.1 -- this is a basis for dt is other functions to advance time.

-- Particle: position velocity
data Ptcl = Ptcl {
    mass     :: Ft
  , position :: Ft
  , velocity :: Ft
  , dbg :: Ft
} deriving (Show)

puts = putStrLn -- shorthand
