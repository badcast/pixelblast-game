#pragma once
#include <array>
#include <utility>
#include <memory>

#include <QList>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

#include "PixelBegin.h"

struct PB_EXPORT BlockObject
{
    int x;
    int y;
    int idx;

    BlockObject(int x, int y, int idx = -1) : x(x), y(y), idx(idx)
    {
    }

    QPointF adjustPoint(const QPointF &adjust, const QSizeF &scale) const;
};

struct ShapeBlock
{
    int shapeColor;
    int rows;
    int columns;
    QList<std::uint8_t> rawBlocks;
    QList<BlockObject> blocks;
};

struct BlockResource
{
    QString name;
    QList<QPixmap> resources;
};

struct PGlobalResources
{
    std::shared_ptr<QPixmap> gameLogo {};
    std::shared_ptr<QPixmap> gridCell {};
    std::shared_ptr<QPixmap> gridCellBright {};
    std::shared_ptr<QPixmap> backgroundPix {};
    std::shared_ptr<QPixmap> cursorPix {};
    std::shared_ptr<QPixmap> gridBackgroundBorder {};
    std::shared_ptr<QPixmap> gridBackground {};
    std::shared_ptr<QPixmap> gridBackgroundBg {};
    std::shared_ptr<QPixmap> uiTopHeader {};
    std::shared_ptr<QList<BlockResource>> BlockRes {};
    std::shared_ptr<SoundManager> soundManager {};
};

class PB_EXPORT PixelBlast : public QWidget
{
    Q_OBJECT

public:
    PixelBlast(QWidget *parent = nullptr);

    void startGame();
    void stopGame();
    void resetGame();

    bool isPlaying();

    inline int getFrames()
    {
        return frames;
    }

    inline int getScores()
    {
        return scores;
    }

signals:
    void endOfGame();

private slots:
    void updateScene();

    // void receiveCurrent(const PixelStats &stat, bool ok);
    // void receiveStats(const QList<PixelStats> &stats, bool ok);

private:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void updateData();
    bool canTrigger(const QList<std::uint8_t> &blocks, QList<std::uint8_t> &grids, bool placeTo = false);
    void generateCandidates(bool randomOnly);
    void assignBlocks(const QList<std::uint8_t> &blocks, ShapeBlock &assignTo);
    QList<std::uint8_t> createBlocks(int shape);

    float heightOffsetCandidates = 30;

    int cellSquare;
    int mouseBtn;
    int scores;
    int frames;
    int frameIndex;
    int round;
    int lastSelectedBlock;

    bool mouseDownMode;
    bool mouseDownUpped;

    QPoint mousePoint;
    QSizeF cellScale;
    QSizeF cellSize;
    QSizeF scaleFactor;
    QRectF boardRegion;
    QTimer updateTimer;
    QList<std::uint8_t> grid;

    QList<PixelStats> _onlineStats;

    float destroyScaler;
    QList<std::pair<BlockObject, int>> destroyBlocks;

    int shapeCandidateIdx;
    std::array<std::shared_ptr<ShapeBlock>, 3> shapeCandidates;

    std::shared_ptr<ShapeBlock> currentShape;

    std::shared_ptr<PGlobalResources> _res;
};
