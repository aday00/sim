--module Gravity (Ptcl(mass, position, velocity, dbg), fall_twin') where
module Gravity where

import System.IO
import Util

-- return new particle effected by gravity
-- f = ma => f / m = a => f/m = dv/dt => (f*dt)/m = dv
gravitate_earth :: Ptcl -> Ptcl
gravitate_earth p = p' where  
  gravity_earth = 9.8
  dt  = deltatime
  dv  = (gravity_earth * dt) / (mass p)
  vel = (velocity p) + dv
  p'  = Ptcl {
      mass     = mass p
    , position = (position p) - (vel * dt)
    , velocity = vel
    , dbg = 0
  } 

-- gravitate until hit the ground
fall_earth :: [Ptcl] -> [Ptcl]
fall_earth ptcls@(p:ps) | position p <= 0 = ptcls
fall_earth ptcls@(p:ps) = fall_earth (p':ptcls) where
  p' = gravitate_earth p

-- two particles gravitate toward each other
-- f = big_g * (m1 * m2) / r^2 = ma
-- dv = (f*dt)/m, and masses m1 and m2 may have differing dv
gravitate_twin :: (Ptcl, Ptcl) -> (Ptcl, Ptcl)
gravitate_twin (a, b) = (a', b') where
  grav_const = 6.67428e-11 -- big g
  dt  = deltatime
  ma  = mass a
  mb  = mass b
  sa  = position a
  sb  = position b
  va  = velocity a
  vb  = velocity b
  f   = grav_const * (ma * mb) / ((sb - sa)^2)
  dir = if sa <= sb then 1 else -1 -- direction of forces
  fdt = dir * f * dt -- a moves right to b when dir = 1, else left
  dva = fdt / ma
  dvb = fdt / mb
  va' = va + dva
  vb' = vb - dvb
  a' = Ptcl {
      mass     = ma
    , position = sa + (va' * dt)
    , velocity = va'
    , dbg = fdt
  }
  b' = Ptcl {
      mass     = mb
    , position = sb + (vb' * dt)
    , velocity = vb'
    , dbg = fdt
  }

-- gravitate until positional convergence (little movement, similar position)
fall_twin_convergent_velocity = 0.1 -- fixme: more appropo measure, dt-based?
fall_twin_convergent_position = 0.1
fall_twin_iteration_limit = 10000
fall_twin :: Int -> [(Ptcl, Ptcl)] -> [(Ptcl, Ptcl)]
fall_twin iter pptcls | iter >= fall_twin_iteration_limit = pptcls
--fall_twin iter pptcls@(pp:pps)
--  | convergent_velocity pa
--  , convergent_velocity pb
--  , abs(abs(position pa) - abs(position pb)) <= fall_twin_convergent_position
--  = pptcls where -- converged in velocities and relative positions
--  pa = fst pp
--  pb = snd pp
--  convergent_velocity p = abs(velocity p) <= fall_twin_convergent_velocity
fall_twin iter pptcls = fall_twin (iter + 1) (pptcls ++ [pp']) where
  pp  = last pptcls
  pp' = gravitate_twin pp

fall_twin' :: Int -> (Ptcl, Ptcl) -> IO ()
fall_twin' iter pp | iter >= fall_twin_iteration_limit = (putStrLn . show) $ pp
--fall_twin' iter pp
--  | convergent_velocity pa
--  , convergent_velocity pb
--  , abs(abs(position pa) - abs(position pb)) <= fall_twin_convergent_position
--  = (putStrLn . show) $ pp where -- converged in velocities and relative positions
--  pa = fst pp
--  pb = snd pp
--  convergent_velocity p = abs(velocity p) <= fall_twin_convergent_velocity
fall_twin' iter pp = do
  putStrLn $ (show iter) ++ ": " ++ (show pp)
  fall_twin' (iter + 1) (gravitate_twin pp)

