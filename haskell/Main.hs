{-# LANGUAGE RecordWildCards #-}
module Main where

import Control.Monad
import Data.Map (Map)
import qualified Data.Map as Map

data Action = Action {
    act_x :: Int
  , act_y :: Int
  } deriving(Show,Eq,Ord)

data MoveResult = Draw | Win | Continue
  deriving(Show,Eq,Ord)

data Cell = X | O | E
  deriving(Show,Ord,Eq,Enum,Bounded)

data Board = Board {
    bo_cells :: Map Int Cell
  } deriving(Show,Ord,Eq)

board_ny = 3
board_nx = 3
(.+) (a,b) (c,d) = (a+c,b+d)
(.*) x (a,b) = (x*a,x*b)
board_inside (x,y) = x >=0 && x < board_nx && y >=0 && y < board_ny
board_winsize = 3
board_points = [(x,y) | x<-[0..board_nx-1], y<-[0..board_ny-1]]
board_dirs = [(0,1),(1,0),(1,1),(1,-1)]
board_rows = filter ( and . map board_inside ) $ [ map (\x -> p .+ (x .* d)) [0..board_winsize-1] | p <- board_points, d <- board_dirs]
board_wincheck = Map.fromList [ (p,filter (p`elem`) board_rows) | p <- board_points ]
board_pos (x,y) = (y*board_ny + x)

emptyBoard = Board $ Map.fromList $ (map board_pos board_points)`zip`(repeat E)

showBoard b =
  forM_ [0..board_ny-1] $ \y -> do
    forM_ [0..board_nx-1] $ \x -> putStr $ show (at_xy b (x,y))
    putStrLn ""


move :: Board -> Action -> Cell -> (Board, MoveResult)
move b@Board{..} a@Action{..} who
  | b`at`a == E && who /= E =
    let
      p = (act_x,act_y)
      bo_cells' = Map.insert (board_pos p) who bo_cells
      res =
        case Map.size bo_cells' >= board_ny * board_nx  of
          True -> Draw
          False ->
            case or $ map (all (==who) . map (at_xy b)) $ board_wincheck Map.! p of
              True -> Win
              False -> Continue
    in
    (Board bo_cells', res)
  | otherwise = error "move: invalid move"


at :: Board -> Action -> Cell
at b Action{..} = b`at_xy`(act_x,act_y)

at_xy :: Board -> (Int,Int) -> Cell
at_xy Board{..} p = bo_cells Map.! (board_pos p)

data Policy = Policy {
    po_map :: Map Board Action
  } deriving(Show,Eq,Ord)

data Avg = Avg Double
  deriving(Show,Ord,Eq)

data Q = Q {
    q_val :: Map (Board, Action) Avg
  } deriving(Show,Ord,Eq)




