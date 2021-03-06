/*
 *	sticker.h - Sticker class
 *	AYM 2000-07-06
 */


class Img;
class Sticker_priv;


class Sticker
{
  public :
    Sticker ();
    Sticker (const Img& img, bool opaque);
    ~Sticker ();
    bool is_clear ();
    void clear ();
    void load (const Img& img, bool opaque);
    void draw (Drawable drw, char grav, int x, int y);

  private :
    Sticker (const Sticker&);			// Too lazy to implement it
    Sticker& operator= (const Sticker&);	// Too lazy to implement it
    Sticker_priv *priv;
};

