{-# LANGUAGE NoImplicitPrelude #-}
module Data.Ix where

range     (start,end) = [start..end]
index     (start,end) val = val - start
inRange   (start,end) val = start <= val && val <= end
rangeSize (start,end) = end - start + 1
