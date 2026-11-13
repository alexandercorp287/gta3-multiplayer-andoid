#ifndef CSPAWNSCREEN_H
#define CSPAWNSCREEN_H

class CSpawnScreen {
public:
    CSpawnScreen();
    ~CSpawnScreen();

    void Show(bool bShow) { m_bEnabled = bShow; }
    void Draw();

private:
    bool m_bEnabled;
};

extern CSpawnScreen* pSpawnScreen;

#endif
