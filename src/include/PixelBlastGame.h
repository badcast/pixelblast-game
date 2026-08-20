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

    void setCellSize(float newSize);
    void startGame();
    void stopGame();
    void resetGame();

    bool isPlaying();

    inline int getFrames()
    {
        return mFrames;
    }

    inline int getScores()
    {
        return mScores;
    }

signals:
    void endOfGame();

private slots:
    void updateScene();

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

    int mCellSquare;
    int mMouseBtn;
    int mScores;
    int mFrames;
    int mFrameIndex;
    int mRound;
    int mLastSelectedBlock;

    bool mMouseDownMode;
    bool mMouseDownUpped;

    QPoint mMousePoint;
    QSizeF mCellScale;
    QSizeF mCellSize;
    QSizeF mScaleFactor;
    QRectF mBoardRegion;
    QTimer mUpdateTimer;
    QList<std::uint8_t> mGrid;

    QList<PixelStats> mOnlineStats;

    float mDestroyScaler;
    QList<std::pair<BlockObject, int>> mDestroyBlocks;

    int shapeCandidateIdx;
    std::array<std::shared_ptr<ShapeBlock>, 3> mShapeCandidates;

    std::shared_ptr<ShapeBlock> mCurrentShape;

    std::shared_ptr<PGlobalResources> mRsrc;
};
