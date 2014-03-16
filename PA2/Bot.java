
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Scanner;
import java.util.StringTokenizer;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author stefan
 */
public class Bot {
    
    public static final int MAXDEPTH = 7;
    public static final int INF = Status.WON;
    
    /*intoarce urmatoarea directie in care mut*/
    public static int findNextMove(Pitch teren) {
        
        ArrayList<Integer> bestMoves = new ArrayList();
        int bestScore = -INF;
        int score;
        /*directiile valide in care pot muta din pozitia curenta*/
        ArrayList<Integer> validMoves = teren.getDirections();
        
        /*daca s-a mai trecut prin punctul curent
         * trebuie sa fac tot eu urmatoarea mutare
         */
        while(!validMoves.isEmpty()) {
            int dir = validMoves.get(0);
            Pair next = teren.nextPosition(dir);
            /*daca trebuie sa mut tot eu*/
            if(teren.pos[next.first][next.second].makeAnotherMove()) {
                teren.markTowards(dir, false);
                score = Max(teren, 0);
                
            }
            /*daca urmeaza adversarul la mutare*/
            else {
                teren.markTowards(dir, false);
                score = Min(teren, 0);
                
            }
            if(score > bestScore) {
                    bestScore = score;
                    bestMoves.clear();
                    bestMoves.add(dir);
             }
            else if(score == bestScore) {
                bestMoves.add(dir);
            }
            /*refac starea terenului*/
            teren.markTowards(opposite(dir), true);
            validMoves.remove(0);
            
        }
        return bestMoves.get(0);
            
    }
    
    public static int Min(Pitch teren, int depth) {
        
        int value = teren.evaluate();
        if(value != Status.PLAYING) {
            return value;
        }
        if(depth == MAXDEPTH) {
            return -heuristic(teren);
        }
        ArrayList<Integer> validMoves = teren.getDirections();
        int score;
        int bestScore = INF;
        while(!validMoves.isEmpty()) {
            int dir = validMoves.get(0);
            Pair next = teren.nextPosition(dir);
            /*daca trebuie sa mute tot adversarul*/
            if(teren.pos[next.first][next.second].makeAnotherMove()) {
                teren.markTowards(dir, false);
                score = Min(teren, depth + 1);  
            }
            else {
                teren.markTowards(dir, false);
                score = Max(teren, depth + 1);
            }
            if(score < bestScore) {
                bestScore = score;
            }
            /*refac starea terenului*/
            teren.markTowards(opposite(dir), true);
            validMoves.remove(0);
        }
        return bestScore;
    }
    public static int Max(Pitch teren, int depth) {
        
        int value = teren.evaluate();
        if(value != Status.PLAYING) {
            return value;
        }
        if(depth == MAXDEPTH) {
            return heuristic(teren);
        }
        ArrayList<Integer> validMoves = teren.getDirections();
        int score;
        int bestScore = -INF;
        while(!validMoves.isEmpty()) {
            int dir = validMoves.get(0);
            Pair next = teren.nextPosition(dir);
            /*daca trebuie sa mut tot eu*/
            if(teren.pos[next.first][next.second].makeAnotherMove()) {
                teren.markTowards(dir, false);
                score = Max(teren, depth + 1);    
            }
            /*daca urmeaza adversarul la mutare*/
            else {
                teren.markTowards(dir, false);
                score = Min(teren, depth + 1);
                
            }
            if(score > bestScore) {
                bestScore = score;
            }
            /*refac starea terenului*/
            teren.markTowards(opposite(dir), true);
            validMoves.remove(0);
        }
        return bestScore;
    }
    
    /*functia de euristica*/
    public static int heuristic(Pitch teren) {
        
        int i = teren.current_pos.first;
        int j = teren.current_pos.second;
        
        if((j == 1) || (j == 7)) {
            return i + 1;
        }
        
        else if((j == 0) || (j == 8)) {
            return i + 2;
        }
        /*daca am lovit bara portii adversarului*/
        else if((i == 11) && ((j == 3) || (j == 5))) {
            return INF / 2;
        }
        /*daca am lovit bara portii mele*/
        else if((i == 1) && ((j == 3) || (j == 5))) {
            return -INF/2;
        }
        /*daca ma aflu in fata portii mele*/
        else if((i == 2) && ((j >= 2) && (j <= 6))) {
            return -INF / 4;
        }
        else {
            return i + 3;
        }
    }
    /*intoarce opusul unei directii*/
    public static int opposite(int dir) {
        int d = -1;
        if(dir == Coord.NORTH) {
            d = Coord.SOUTH;
        }
        else if(dir == Coord.SOUTH) {
            d = Coord.NORTH;
        }
        else if(dir == Coord.EAST) {
            d = Coord.WEST;
        }
        else if(dir == Coord.WEST) {
            d = Coord.EAST;
        }
        else if(dir == Coord.NE) {
            d = Coord.SW;
        }
        else if(dir == Coord.SE) {
            d = Coord.NW;
        }
        else if(dir == Coord.NW) {
            d = Coord.SE;
        }
        else if(dir == Coord.SW) {
            d = Coord.NE;
        }
        return d;
    }
    
    public static void main(String[] args) throws IOException {
        
        Pitch teren = new Pitch();
        InputStreamReader in = new InputStreamReader(System.in);
        BufferedReader br = new BufferedReader(in);
        int i, j, no_of_moves, move, no;
        
        while(true) {
            /*citesc comanda primita*/
            String com = br.readLine();
            StringTokenizer st = new StringTokenizer(com);
            if(!st.hasMoreTokens()) {
            	break;
            }
            String letter = st.nextToken();
            /*daca s-a incheiat jocul*/
            if(letter.equals("F")) {
                break;
            }
            /*daca am primit mutare de la adversar*/
            if(letter.equals("M")) {
                /*retin numarul de mutari*/
                no_of_moves = Integer.parseInt(st.nextToken());
                /*marchez mutarile adversarului pe teren*/
                for(i = 0; i < no_of_moves; i++) {
                    move = Integer.parseInt(st.nextToken());
                    teren.markTowards(move, false);
                }
            }
            /*daca eu sunt la mutare*/
           
            /*gasesc urmatoarea mutare*/
            no = 1;
            String my_moves = "";
            my_moves += "M";
            move = findNextMove(teren);
            my_moves += " " + move;
            Pair next = teren.nextPosition(move);
            i = next.first;
            j = next.second;
            boolean another = teren.pos[i][j].makeAnotherMove();
            teren.markTowards(move, false);
            while(another) {
                move = findNextMove(teren);
                no ++;
                my_moves += " " + move;
                next = teren.nextPosition(move);
                i = next.first;
                j = next.second;
                another = teren.pos[i][j].makeAnotherMove();
                teren.markTowards(move, false);
            }
            my_moves = my_moves.substring(0, 2) + no 
                                    + my_moves.substring(1, my_moves.length());
            System.out.println(my_moves);
        }
    }
    
    
}
