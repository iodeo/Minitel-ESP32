#ifndef MINICHESS_H
#define MINICHESS_H

#include <Minitel1B_Hard.h>

#define CASE_WIDTH 4
#define CASE_HEIGHT 3
#define BOARD_TOP 1
#define BOARD_LEFT 5
#define PIECE_WIDTH 3
#define PIECE_HEIGHT 3

#define COLOR_MASK 0b1000
#define PIECE_MASK 0b0111

class MiniChess {
  public:
    MiniChess(HardwareSerial& serial); // constructeur définissant l'objet "minitel"
    void initializeMinitel(int bauds); // réglage port série et config minitel
    void drawChessBoard();             // affichage plateau vide
    void initPiecesPosition();         // placement des pièces dans le tableau board
    void drawAllPieces();              // affichage des pièces en se basant sur board

    void getMove();      // saisie du mouvement par l'utilisateur
    void makeMove();     // affichage mouvement incluant les mouvements spéciaux
    
    void changePlayer(); // changement de joueur

  private:
    void drawPiece(int cx, int cy, byte pc);  // affichage d'une pièce dans une case donnée
    void erasePiece(int cx, int cy);          // effacement de la pièce d'une case donnée
    
    void markCase(int cx, int cy, bool mark); // marquage de position et sélection
    void markCase(int cx, int cy, bool mark, bool force_erase); // effacement selection
    
    void moveUp();    // déplacement du curseur de selection vers le haut
    void moveDown();  // déplacement .. vers le bas
    void moveLeft();  // déplacement .. vers la gauche
    void moveRight(); // déplacement .. vers la droite
    
    bool validateSelection(); // valide la selection de la 1ere case
    bool validateMove();      // valide la 2e case
    
    void drawMove(); // affichage mouvement simple
    
    unsigned long getKeyboardInput(); // gère les saisies clavier
    
    // Instance Minitel (voir Minitel1B_Hard)
    Minitel minitel;

    // Identification des pièces de jeux
    enum { VOID, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING}; // PIECES
    enum {_BLACK = 0b0000, _WHITE = 0b1000}; // COLORS
    
    // Pieces en 3*3 caractères semi-graphiques décrits dans le sens de la lecture
    byte piece[7][PIECE_WIDTH*PIECE_HEIGHT] = { 
      {0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000}, // VOID
      {0b000000, 0b000000, 0b000000, 0b000101, 0b101111, 0b000000, 0b000100, 0b101100, 0b000000}, // PAWN
      {0b000010, 0b000010, 0b000010, 0b110101, 0b111101, 0b100000, 0b011100, 0b011100, 0b001000}, // ROOK
      {0b000000, 0b000111, 0b000010, 0b011110, 0b011101, 0b101010, 0b001100, 0b111100, 0b001000}, // KNIGHT
      {0b000001, 0b001011, 0b000000, 0b111111, 0b101111, 0b101010, 0b011100, 0b111100, 0b001000}, // BISHOP
      {0b001001, 0b000011, 0b001000, 0b000111, 0b101111, 0b000010, 0b111100, 0b011100, 0b101000}, // QUEEN
      {0b000001, 0b001011, 0b000000, 0b000111, 0b101111, 0b000010, 0b111100, 0b011100, 0b101000} // KING
    };
    
    // tableau des cases du plateau
    byte board[8][8] {};
    
    // position courante sur le plateau
    int cx = 0; // 0-7 > A-H (de gauche à droite)
    int cy = 7; // 0-7 > 8-1 (de haut en bas)
    
    // sélection courante
    int scx = -1;
    int scy = -1;
    
    // joueur actif
    byte player = _WHITE;

};

#endif
