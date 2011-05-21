#!/usr/bin/env ruby

def term(x)
  1.0 / ((x << 1) + 1)
end

def pi(lim)
  acc = 0
  coeff = 0
  0.upto(lim) do |i|
    coeff = (i & 1 == 0) ? (1) : (-1)
    acc = acc + (term(i)*coeff)
  end
  return (4 * acc)
end

def pi2(lim)
  acc = 0
  coeff = 0
  0.upto(lim) do |i|
    coeff = (i & 1 == 0) ? (1) : (-1)
    acc = acc + (term(i)*coeff)
  end
  return 4 * (acc + (-coeff * (term(lim)/2.0) ))
end

def sgn(x)
  (x & 1 == 0) ? (1) : (-1)
end

def sgnf(x)
  (x & 1 == 0) ? (1.0) : (-1.0)
end
def term2(x)
  sgnf(x) / ((x << 1) + 1)
end


def pi3(lim)
  acc = 0
  0.upto(lim - 1) do |i|
    acc = acc + term2(i)
  end
  return (4 * acc) + (2 * term2(lim))
end

start = 0
width = 8
start.upto(start + width) do |i|
  printf "%8d  %1.10f\n", i, pi(i)
end
printf "      pi  %1.10f\n", Math::PI
puts

puts ((pi(5) + pi(6))/2.0)

puts
start.upto(start + width) do |i|
  printf "%8d  %1.10f\n", i, pi2(i)
end

puts
start.upto(start + width) do |i|
  printf "%8d  %1.10f\n", i, pi3(i)
end

exit
start = 100000
width = 6
start.upto(start + width) do |i|
  printf "%8d  %1.10f\n", i, pi(i)
end
printf "      pi  %1.10f\n", Math::PI
