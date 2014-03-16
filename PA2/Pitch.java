
import java.util.ArrayList;

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/**
 *
 * @author stefan
 */
public class Pitch {
    /*retine informatii despre toate pozitiile din teren*/
    public Position[][] pos;
    /*retine pozitia curenta din teren*/
    public Pair current_pos;
    
    public Pitch() {
        /*initializare teren*/
        int i, j;
        pos = new Position[13][9];
        for(i = 0; i < 13; i++) {
            for(j = 0; j < 9; j++) {
                pos[i][j] = new Position();
            }
        }
        /*marchez limitele terenului*/
        /*west side*/
        for(i = 1; i <= 11; i++) {
            /*north marked as inaccessable*/
            pos[i][0].markDirection(Coord.NORTH, false);
            /*south marked as inaccessable*/
            pos[i][0].markDirection(Coord.SOUTH, false);
            /*west marked as inaccessable*/
            pos[i][0].markDirection(Coord.WEST, false);
            /*south-west*/
            pos[i][0].markDirection(Coord.SW, false);
            /*north-west*/
            pos[i][0].markDirection(Coord.NW, false);
            
            pos[i][0].setAsMargin();
        }
        /*east side*/
        for(i = 1; i <= 11; i++) {
             /*north marked as inaccessable*/
            pos[i][8].markDirection(Coord.NORTH, false);
            /*south marked as inaccessable*/
            pos[i][8].markDirection(Coord.SOUTH, false);
            /*east marked as inaccessable*/
            pos[i][8].markDirection(Coord.EAST, false);
            /*north-east*/
            pos[i][8].markDirection(Coord.NE, false);
            /*south-east*/
            pos[i][8].markDirection(Coord.SE, false);
            
            pos[i][8].setAsMargin();
        }
        /*south side*/
        for(j = 0; j <= 8; j++) {
            if(!((j >= 3) && (j <= 5))) {
                /*east marked as inaccessable*/
                pos[1][j].markDirection(Coord.EAST, false);
                /*south marked as inaccessable*/
                pos[1][j].markDirection(Coord.SOUTH, false);
                /*west marked as inaccessable*/
                pos[1][j].markDirection(Coord.WEST, false);
                /*south-east*/
                pos[1][j].markDirection(Coord.SE, false);
                /*south-west*/
                pos[1][j].markDirection(Coord.SW, false);
                
                pos[1][j].setAsMargin();
            }
        }
        /*set the posts for my goalpost*/
        pos[0][3].setAsGoal();
        pos[0][4].setAsGoal();
        pos[0][5].setAsGoal();
        /*left post*/
        pos[1][3].setAsMargin();
        pos[1][3].markDirection(Coord.SOUTH, false);
        pos[1][3].markDirection(Coord.WEST, false);
        pos[1][3].markDirection(Coord.SW, false);
        /*right post*/
        pos[1][5].setAsMargin();
        pos[1][5].markDirection(Coord.SOUTH, false);
        pos[1][5].markDirection(Coord.EAST, false);
        pos[1][5].markDirection(Coord.SE, false);
        /*north side*/
        for(j = 0; j <= 8; j++) {
            if(!((j >= 3) && (j <= 5))) {
               /*east marked as inaccessable*/
                pos[11][j].markDirection(Coord.EAST, false);
                /*south marked as inaccessable*/
                pos[11][j].markDirection(Coord.NORTH, false);
                /*west marked as inaccessable*/
                pos[11][j].markDirection(Coord.WEST, false);
                /*north-east*/
                pos[11][j].markDirection(Coord.NE, false);
                /*north-west*/
                pos[11][j].markDirection(Coord.NW, false);
                
                pos[11][j].setAsMargin();
            }
        }
        /*set the posts for my oponent goalpost*/
        pos[12][3].setAsGoal();
        pos[12][4].setAsGoal();
        pos[12][5].setAsGoal();
        /*left post*/
        pos[11][3].setAsMargin();
        pos[11][3].markDirection(Coord.NORTH, false);
        pos[11][3].markDirection(Coord.WEST, false);
        pos[11][3].markDirection(Coord.NW, false);
        /*right post*/
        pos[11][5].setAsMargin();
        pos[11][5].markDirection(Coord.NORTH, false);
        pos[11][5].markDirection(Coord.EAST, false);
        pos[11][5].markDirection(Coord.NE, false);
        
        /*Corners*/
        pos[2][1].SWCorner = true;
        pos[2][1].markDirection(Coord.SW, false);
        pos[10][1].NWCorner = true;
        pos[10][1].markDirection(Coord.NW, false);
        pos[2][7].SECorner = true;
        pos[2][7].markDirection(Coord.SE, false);
        pos[10][7].NECorner = true;
        pos[10][7].markDirection(Coord.NE, false);
        
        /*pozitia initiala este in centrul terenului*/
        current_pos = new Pair(6,4);
    }
    
    /*intoarce noua pozitie pe teren in urma mutarii*/
    public void markTowards(int coord, boolean how) {
        /*mutare spre nord*/
        int i = current_pos.first;
        int j = current_pos.second;
        
        
            if(coord == Coord.NORTH) {
                pos[i][j].markDirection(Coord.NORTH, how);
                pos[i+1][j].markDirection(Coord.SOUTH, how);
                /*actualizez pozitia curenta*/
                current_pos = nextPosition(Coord.NORTH);
            }
            /*mutare spre sud*/
            else if(coord == Coord.SOUTH) {
                pos[i][j].markDirection(Coord.SOUTH, how);
                pos[i-1][j].markDirection(Coord.NORTH, how);
                current_pos = nextPosition(Coord.SOUTH);
            }
            else if(coord == Coord.WEST) {
                pos[i][j].markDirection(Coord.WEST, how);
                pos[i][j-1].markDirection(Coord.EAST, how);
                current_pos = nextPosition(Coord.WEST);
            }
            else if(coord == Coord.EAST) {
                pos[i][j].markDirection(Coord.EAST, how);
                pos[i][j+1].markDirection(Coord.WEST, how);
                current_pos = nextPosition(Coord.EAST);
            }
            else if(coord == Coord.NE) {
                pos[i][j].markDirection(Coord.NE, how);
                pos[i+1][j+1].markDirection(Coord.SW, how);
                current_pos = nextPosition(Coord.NE);
            }
            else if(coord == Coord.NW) {
                pos[i][j].markDirection(Coord.NW, how);
                pos[i+1][j-1].markDirection(Coord.SE, how);
                current_pos = nextPosition(Coord.NW);
            }
            else if(coord == Coord.SE) {
                pos[i][j].markDirection(Coord.SE, how);
                pos[i-1][j+1].markDirection(Coord.NW, how);
                current_pos = nextPosition(Coord.SE);
            }
            else if(coord == Coord.SW) {
                pos[i][j].markDirection(Coord.SW, how);
                pos[i-1][j-1].markDirection(Coord.NE, how);
                current_pos = nextPosition(Coord.SW);
            }
        
    }
    
    /*intoarce urmatoare pozitie pe teren daca s-ar face mutarea
     * spre coordonata coord
     */
    public Pair nextPosition(int coord) {
        Pair next = new Pair(-1, -1);
        if(coord == Coord.NORTH) {
            next = new Pair(current_pos.first+1, current_pos.second);
        }
        else if(coord == Coord.SOUTH) {
            next = new Pair(current_pos.first - 1, current_pos.second);
        }
        else if(coord == Coord.WEST) {
            next = new Pair(current_pos.first, current_pos.second - 1);
        }
        else if(coord == Coord.EAST) {
            next = new Pair(current_pos.first, current_pos.second + 1);
        }
        else if(coord == Coord.NE) {
            next = new Pair(current_pos.first + 1, current_pos.second + 1);
        }
        else if(coord == Coord.NW) {
            next = new Pair(current_pos.first + 1, current_pos.second - 1);
        }
        else if(coord == Coord.SE) {
            next = new Pair(current_pos.first - 1, current_pos.second + 1);
        }
        else if(coord == Coord.SW) {
            next = new Pair(current_pos.first - 1, current_pos.second - 1);
        }
        return next;
    }
    
    public int evaluate() {
        int i = current_pos.first;
        int j = current_pos.second;
        /*daca s-a marcat gol*/
        if((j == 3) || (j == 4) || (j ==5)) {
            if(i == 12) {
                return Status.WON;
            }
            if(i == 0) {
                return Status.LOST;
            }
        }
        return Status.PLAYING;
    }
    
    public ArrayList<Integer> getDirections() {
        
        int i = current_pos.first;
        int j = current_pos.second;
        return pos[i][j].getAccDirections();
    }
    
    
}
