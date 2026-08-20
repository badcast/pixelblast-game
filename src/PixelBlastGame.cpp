#include <cstdint>
#include <cstring>
#include <utility>
#include <memory>
#include <stdexcept>
#include <functional>

#include <QFile>
#include <QRandomGenerator>
#include <QPainter>
#include <QBrush>
#include <QPalette>
#include <QMouseEvent>
#include <QTextStream>
#include <QMessageBox>
#include <QCursor>

#include "PixelBlastGame.h"
#include "PixelBlastShapes.h"
#include "PixelNetwork.h"
#include "PixelSoundManager.h"

constexpr int MaxCellWidth = 8;

std::shared_ptr<PGlobalResources> _resource;

QPixmap adjustBright(const QPixmap &pixmap, int brightness)
{
    QImage img = pixmap.toImage();
    QColor color;
    int x, y;
    for(x = 0; x < img.width(); ++x)
    {
        for(y = 0; y < img.height(); ++y)
        {
            color = std::move(img.pixelColor(x, y));
            color.setRed(qBound(0, color.red() + brightness, 255));
            color.setGreen(qBound(0, color.green() + brightness, 255));
            color.setBlue(qBound(0, color.blue() + brightness, 255));
            img.setPixelColor(x, y, color);
        }
    }
    return QPixmap::fromImage(img);
}

template <typename InT, typename OutT>
constexpr inline OutT map(const InT x, const InT in_min, const InT in_max, const OutT out_min, const OutT out_max)
{
    if(in_max == in_min)
    {
        return out_min;
    }
    OutT mapped_value = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    return qMin(qMax(mapped_value, out_min), out_max);
}

inline QPixmap *getColoredPixmap(int color, int frameIndex, std::shared_ptr<PGlobalResources> &_res)
{
    BlockResource *br = &(*_res->BlockRes)[color];
    return &(br->resources[frameIndex % br->resources.size()]);
}

inline void drawShapeAt(const ShapeBlock &shape, QPointF destPoint, QSizeF size, int frameIndex, QPainter &p, std::shared_ptr<PGlobalResources> &_res, QPixmap *pixmap = nullptr)
{
    int x;
    QRectF dest;
    dest.setSize(size);
    if(pixmap == nullptr)
        pixmap = getColoredPixmap(shape.shapeColor, frameIndex, _res);
    for(x = 0; x < shape.blocks.size(); ++x)
    {
        dest.moveTopLeft(shape.blocks[x].adjustPoint(destPoint, dest.size()));
        p.drawPixmap(dest, *pixmap, {});
    }
};

void prepareResources()
{
    // Load and initialize globalResources
    if(_resource)
        return;

    constexpr auto _formatResourceName = ":/pixelblastgame/resourcepacks/blocks/%s";
    constexpr auto _formatBlocks = "%s %d %s";
    constexpr auto MaxBufLen = 128;

    int n;
    char buff[MaxBufLen], buff0[64], buff1[64];

    BlockResource tmp;
    QString content, bfName;
    std::snprintf(buff, MaxBufLen, _formatResourceName, "blocks.cfg");
    QFile file(buff);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        throw std::runtime_error("Resource is not access");
        return;
    }

    _resource = std::make_shared<PGlobalResources>();

    QTextStream stream(&file);
    _resource->BlockRes = std::make_shared<QList<BlockResource>>();
    while(stream.readLineInto(&content))
    {
        if(sscanf((content).toLocal8Bit().data(), _formatBlocks, buff, &n, buff0) != 3)
        {
            _resource->BlockRes.reset();
            return;
        }
        tmp.name = buff;
        for(int i = 0; i < n; ++i)
        {
            snprintf(buff, MaxBufLen, _formatResourceName, buff0);
            content = buff;
            content.replace(QChar('#'), QString::number(i + 1));
            QPixmap qp(content);
            tmp.resources.append(std::move(qp));
        }
        _resource->BlockRes->append(std::move(tmp));
    }
    _resource->gameLogo = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/game-logo")));
    _resource->uiTopHeader = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/ui-top")));
    _resource->backgroundPix = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/background")));
    _resource->cursorPix = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/arrow")));
    _resource->gridBackgroundBorder = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/grid-border")));
    _resource->gridBackground = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/grid-background")));
    _resource->gridBackgroundBg = std::make_shared<QPixmap>(std::move(QPixmap(":/pixelblastgame/grid-background-bg")));
    _resource->gridCell = std::make_shared<QPixmap>(std::move(adjustBright(QPixmap(":/pixelblastgame/grid-cell"), 40)));
    _resource->gridCellBright = std::make_shared<QPixmap>(std::move(adjustBright(*_resource->gridCell, 70)));

    // Sounds
    _resource->soundManager = std::make_shared<SoundManager>(nullptr);
    _resource->soundManager->setPoolSize(24);
    _resource->soundManager->registerSound("block-hits", QUrl::fromLocalFile(":/pixelblastgame/block-hits"), false);

    _resource->soundManager->registerSound("block-click0", QUrl::fromLocalFile(":/pixelblastgame/block-click0"));
    _resource->soundManager->registerSound("block-click1", QUrl::fromLocalFile(":/pixelblastgame/block-click1"));
    _resource->soundManager->registerSound("block-click2", QUrl::fromLocalFile(":/pixelblastgame/block-click2"));

    _resource->soundManager->registerSound("block-place0", QUrl::fromLocalFile(":/pixelblastgame/block-place0"));
    _resource->soundManager->registerSound("block-place1", QUrl::fromLocalFile(":/pixelblastgame/block-place1"));
    _resource->soundManager->registerSound("block-place2", QUrl::fromLocalFile(":/pixelblastgame/block-place2"));

    _resource->soundManager->registerSound("voice0", QUrl::fromLocalFile(":/pixelblastgame/voice0"));
    _resource->soundManager->registerSound("voice1", QUrl::fromLocalFile(":/pixelblastgame/voice1"));
    _resource->soundManager->registerSound("voice2", QUrl::fromLocalFile(":/pixelblastgame/voice2"));
    _resource->soundManager->registerSound("voice3", QUrl::fromLocalFile(":/pixelblastgame/voice3"));
    _resource->soundManager->registerSound("voice-gameover", QUrl::fromLocalFile(":/pixelblastgame/voice-gameover"));

    _resource->soundManager->registerSound("block-destroy", QUrl::fromLocalFile(":/pixelblastgame/block-destroy"));
}

PixelBlast::PixelBlast(QWidget *parent) : QWidget(parent), mUpdateTimer(this), mBoardRegion(0, 0, 328, 328), mRound(0), mCellScale(1.0F, 1.0F), shapeCandidateIdx(-1), mScores(0), mFrames(0), mFrameIndex(0), mDestroyScaler(0), mMouseDownMode(true), mLastSelectedBlock(-1)
{
    prepareResources();
    mRsrc = _resource;
    if(!mRsrc)
    {
        throw std::runtime_error("prepare resources is invalid init");
    }
    resize(mBoardRegion.size().scaled(mBoardRegion.width() + 50, mBoardRegion.height() + 50, Qt::AspectRatioMode::IgnoreAspectRatio).toSize());

    mCellSquare = MaxCellWidth;
    mGrid.assign(MaxCellWidth * MaxCellWidth, 0);

    setMouseTracking(true);
    mUpdateTimer.setSingleShot(false);
    mUpdateTimer.setInterval(1000.F / 60); // 60 FPS per sec

    QBrush background(*mRsrc->backgroundPix);
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, background);
    setPalette(pal);
    setAutoFillBackground(true);

    QCursor cur(*mRsrc->cursorPix, 0, 0);
    setCursor(cur);

    QObject::connect(&mUpdateTimer, &QTimer::timeout, this, &PixelBlast::updateScene);
}

void PixelBlast::startGame()
{
    resetGame();
    mUpdateTimer.start();
}

void PixelBlast::stopGame()
{
    mUpdateTimer.stop();
}

void PixelBlast::resetGame()
{
    mRound = 0;
    mScores = 0;
    mFrames = 0;
    mFrameIndex = 0;
    shapeCandidateIdx = -1;
    mLastSelectedBlock = -1;
    mCurrentShape.reset();
    mDestroyScaler = 0;
    mDestroyBlocks.clear();
    std::fill(std::begin(mGrid), std::end(mGrid), 0x00000);
    std::fill(std::begin(mShapeCandidates), std::end(mShapeCandidates), nullptr);
}

bool PixelBlast::isPlaying()
{
    return mUpdateTimer.isActive();
}

void PixelBlast::mousePressEvent(QMouseEvent *event)
{
    mMouseBtn = event->button() & 0x3;
}

void PixelBlast::mouseReleaseEvent(QMouseEvent *event)
{
    if(mMouseDownMode)
        mMouseDownUpped = (event->button() & 0x1) > 0;
    mMouseBtn = 0;
}

QList<std::uint8_t> PixelBlast::createBlocks(int shape)
{
    int x, y;
    QList<std::uint8_t> blockArr {};
    blockArr.resize(sizeof(shape) * 8);
    for(x = 0, y = 0; x < blockArr.size(); ++x)
    {
        if((blockArr[x] = (shape >> x) & 0x1))
            y = x;
    }
    blockArr.resize(y + 1);
    return blockArr;
}

// TEST ALGO FOR ROTATE BY CLOCKWISE
// QList<int> rotateClockwise(const QList<int> &blockSrc, int blockWidth = MaxCellWidth)
// {
//     int height, x, y, z, w, d;
//     QList<int> rotated;
//     rotated.resize(blockSrc.size(), 0);
//     height = blockSrc.size() / blockWidth;
//     for(y = 0; y < height; ++y)
//     {
//         for(x = 0; x < blockWidth; ++x)
//         {
//             z = y * blockWidth + x;
//             w = height - 1 - y;
//             d = x * height + w;
//             rotated[d] = blockSrc[z];
//         }
//     }
//     return rotated;
// }

void PixelBlast::assignBlocks(const QList<std::uint8_t> &blocks, ShapeBlock &assign)
{
    int x, y, z;

    assign.rows = 0;
    assign.columns = 0;
    assign.shapeColor = 0;
    assign.rawBlocks.clear();
    assign.blocks.clear();
    if(blocks.empty())
    {
        return;
    }
    for(z = 0; z < blocks.size(); ++z)
    {
        if(blocks[z])
        {
            // get point from matrix
            x = z % mCellSquare;
            y = z / mCellSquare;

            // calcluate counts
            assign.columns = qMax(1, qMax(assign.columns, x + 1));
            assign.rows = qMax(1, qMax(assign.rows, y + 1));
            assign.blocks.emplaceBack(x, y);
        }
    }
    x = (mRsrc->BlockRes == nullptr) ? 0 : mRsrc->BlockRes->size();
    assign.rawBlocks = blocks;
    assign.shapeColor = QRandomGenerator::global()->bounded(0, x);
}

void PixelBlast::resizeEvent(QResizeEvent *event)
{
    updateData();
}

bool PixelBlast::canTrigger(const QList<std::uint8_t> &blocks, QList<std::uint8_t> &grids, bool placeTo)
{
    int x, y, z, w, d;
    bool fits;
    int width = qRound(std::sqrt(grids.size()));
    for(x = 0; x < width; ++x)
    {
        for(y = 0; y < width; ++y)
        {
            fits = true;
            for(z = 0; z < blocks.size(); ++z)
            {
                if(blocks[z] == 0)
                    continue;
                w = x + z % width;
                d = y + z / width;
                if(w < 0 || d < 0 || w >= width || d >= width)
                {
                    fits = false;
                    break;
                }
                w = grids[d * width + w] & 0x1;
                if(blocks[z] == 1 && w == 1)
                {
                    fits = false;
                    break;
                }
            }
            if(fits)
            {
                if(placeTo)
                {
                    for(z = 0; z < blocks.size(); ++z)
                    {
                        if(blocks[z] == 0)
                            continue;
                        w = (y + z / width) * width + (x + z % width);
                        grids[w] |= 1;
                    }
                }
                return true;
            }
        }
    }
    return false;
}

void PixelBlast::generateCandidates(bool randomOnly)
{
    int x, y, z, w;
    QList<std::uint8_t> _virtualGrid = mGrid;
    QList<std::uint8_t> _shapes(MaxShapes, 0);
    QList<int> _candidates(mShapeCandidates.size(), 0);

    for(x = 0; x < MaxShapes; ++x)
        _shapes[x] = x;

    w = static_cast<int>(randomOnly);
    for(x = 0; x < mShapeCandidates.size(); ++x)
    {
        switch(w)
        {
                // SELECTIVE
            case 0:
            {
                std::shuffle(std::begin(_shapes), std::end(_shapes), *QRandomGenerator::global());
                for(z = 0; z < MaxShapes; ++z)
                {
                    y = getShape(_shapes[z]) & 0x7FFFFFFF;
                    if(canTrigger(createBlocks(y), _virtualGrid, true))
                    {
                        break;
                    }
                }
                break;
            }
                // RANDOM
            case 1:
            {
                do
                {
                    y = randomShapes();
                } while(std::any_of(std::begin(_candidates), std::end(_candidates), [y](const auto i) { return i == y; }));
            }
        }

        _candidates[x] = y;
        mShapeCandidates[x] = std::make_shared<ShapeBlock>();
        assignBlocks(createBlocks(y), *mShapeCandidates[x]);
    }
}

void PixelBlast::updateData()
{
    mCellSquare = qRound(sqrt(mGrid.size()));
    mCellSize = mBoardRegion.size() / static_cast<float>(mCellSquare);
    mScaleFactor = {mCellScale.width() * mCellSize.width(), mCellScale.height() * mCellSize.height()};
    mBoardRegion.moveTopLeft({(width() - mBoardRegion.width()) / 2, (height() - mBoardRegion.height()) / 2 + 50});
}

void PixelBlast::updateScene()
{
    int x, y, z, w, d, i;
    QPointF tmp, tmp0;
    QRectF dest;

    mMousePoint = mapFromGlobal(QCursor::pos());

    if(mCurrentShape == nullptr && !std::any_of(std::cbegin(mShapeCandidates), std::cend(mShapeCandidates), [](const auto &t) { return t != nullptr; }))
    {
        ++mRound;
        generateCandidates(false);
    }

    if(mDestroyScaler == 0.0F)
    {
        mDestroyBlocks.clear();
    }

    if(mCurrentShape)
    {
        // Reset old mask
        d = 0;
        for(x = 0; x < mCurrentShape->blocks.size(); ++x)
        {
            y = mCurrentShape->blocks[x].idx;
            if(y != -1)
            {
                mGrid[y] = mGrid[y] & 0x1;
                ++d;
            }
            mCurrentShape->blocks[x].idx = -1;
        }

        // Return selected shape after right click
        if(shapeCandidateIdx != -1 && (mMouseDownMode && mMouseDownUpped && d == 0 || mMouseBtn == Qt::RightButton))
        {
            mShapeCandidates[shapeCandidateIdx] = std::move(mCurrentShape);
            shapeCandidateIdx = -1;
        }
    }

    if(mCurrentShape)
    {
        std::uint64_t cellMask = 0;
        tmp.setX(mMousePoint.x() - static_cast<float>(mCurrentShape->columns * mScaleFactor.width()) / 2);
        tmp.setY(mMousePoint.y() - static_cast<float>(mCurrentShape->rows * mScaleFactor.height()) / 2);
        for(w = 0; w < mCurrentShape->blocks.size(); ++w)
        {
            tmp0 = std::move(mCurrentShape->blocks[w].adjustPoint(tmp, mScaleFactor));
            x = mCellSquare * (tmp0.x() - mBoardRegion.x() + mScaleFactor.width() / 2) / (mBoardRegion.width());
            y = mCellSquare * (tmp0.y() - mBoardRegion.y() + mScaleFactor.height() / 2) / (mBoardRegion.height());
            z = y * mCellSquare + x;
            if(x < 0 || y < 0 || x >= mCellSquare || y >= mCellSquare || (mGrid[z] & 0x3) != 0 || ((cellMask >> z) & 1ULL) == 1ULL)
                break;
            cellMask |= (1ULL << z);
            mCurrentShape->blocks[w].idx = z;
        }
        // verification
        if(w == mCurrentShape->blocks.size())
        {
            d = 2;
            if(mMouseDownMode)
                d = (mMouseDownUpped) ? 1 : d;
            else
                d = (mMouseBtn == Qt::LeftButton) ? 1 : d;

            for(x = 0; x < w; ++x)
            {
                y = mCurrentShape->blocks[x].idx;
                mGrid[y] = d | mCurrentShape->shapeColor << 0x2;
            }

            // Place complete.
            if(d == 1)
            {
                mRsrc->soundManager->playSound(QString("block-place%1").arg(QRandomGenerator::global()->bounded(3)), 0.5);

                // Collect full rows and columns
                QList<int> fullRows;
                QList<int> fullCols;
                for(int r = 0; r < mCellSquare; ++r)
                {
                    bool full = true;
                    for(int c = 0; c < mCellSquare; ++c)
                    {
                        if((mGrid[r * mCellSquare + c] & 0x3) != 1)
                        {
                            full = false;
                            break;
                        }
                    }
                    if(full) fullRows.append(r);
                }
                for(int c = 0; c < mCellSquare; ++c)
                {
                    bool full = true;
                    for(int r = 0; r < mCellSquare; ++r)
                    {
                        if((mGrid[r * mCellSquare + c] & 0x3) != 1)
                        {
                            full = false;
                            break;
                        }
                    }
                    if(full) fullCols.append(c);
                }

                // Clear full rows
                for(int r : fullRows)
                {
                    mScores += mCellSquare;
                    for(int c = 0; c < mCellSquare; ++c)
                    {
                        int cellIdx = r * mCellSquare + c;
                        if((mGrid[cellIdx] & 0x3) == 1)
                        {
                            mDestroyBlocks.append(std::make_pair(BlockObject(c, r, cellIdx), mGrid[cellIdx] >> 2));
                            mGrid[cellIdx] = 0;
                        }
                    }
                    mDestroyScaler = 1.0F;
                }
                // Clear full columns
                for(int c : fullCols)
                {
                    mScores += mCellSquare;
                    for(int r = 0; r < mCellSquare; ++r)
                    {
                        int cellIdx = r * mCellSquare + c;
                        if((mGrid[cellIdx] & 0x3) == 1)
                        {
                            mDestroyBlocks.append(std::make_pair(BlockObject(c, r, cellIdx), mGrid[cellIdx] >> 2));
                            mGrid[cellIdx] = 0;
                        }
                    }
                    mDestroyScaler = 1.0F;
                }

                mCurrentShape = nullptr;

                for(x = 0, y = 0, z = 0; x < mShapeCandidates.size(); ++x)
                {
                    mShapeCandidates[x] && ++y && !canTrigger(mShapeCandidates[x]->rawBlocks, mGrid, false) && ++z;
                }
                if(y == z && z > 0)
                {
                    // GAME OVER
                    mRsrc->soundManager->playSound("voice-gameover", 0.5);
                    // QMessageBox::warning(this, "Game Lost", "Game over!");
                    stopGame();
                    emit endOfGame();
                }
                else if(mDestroyScaler == 1.0F)
                {
                    mRsrc->soundManager->playSound("block-destroy", 0.5);
                    mRsrc->soundManager->playSound(QString("voice%1").arg(QRandomGenerator::global()->bounded(4)), 0.5);
                }
            }
        }
    }
    else if(!mShapeCandidates.empty())
    {
        // Select candidate block by Mouse Click!
        tmp.setX(0);
        tmp.setY(mBoardRegion.height() + heightOffsetCandidates);
        tmp = mMousePoint - (mBoardRegion.topLeft() + tmp);
        dest.setSize((mScaleFactor * (static_cast<float>(mCellSquare) / mShapeCandidates.size())));
        if(tmp.y() < 0 || tmp.y() > dest.height() || tmp.x() < 0 || tmp.x() > dest.width() * mShapeCandidates.size())
            x = -1;
        else
            x = static_cast<int>(tmp.x() / qMax(1.0F, dest.width()));

        shapeCandidateIdx = -1;
        x = qBound<int>(-1, x, mShapeCandidates.size());
        if(!(x < 0 || x == mShapeCandidates.size()))
        {
            shapeCandidateIdx = x;
            if(mShapeCandidates[x] && (mMouseDownMode && mMouseDownUpped || mMouseBtn == Qt::LeftButton))
            {
                mCurrentShape = std::move(mShapeCandidates[x]);
                mRsrc->soundManager->playSound(QString("block-click%1").arg(QRandomGenerator::global()->bounded(3)), 0.8);
            }
        }
    }

    update();
    mFrames++;
    mFrameIndex += mFrames % 5 == 0;
    mDestroyScaler = qBound(0.0F, mDestroyScaler - 0.03F, 1.0F);

    if(mMouseDownUpped)
        mMouseDownUpped = false;
    mMouseBtn = 0x0;
}

void PixelBlast::paintEvent(QPaintEvent *event)
{
    int x, y, z, w;
    QRectF dest;
    QPointF destPoint;
    QPainter p(this);
    QPixmap *pixmap;

    QWidget::paintEvent(event);

    // Draw game logo
    dest.setSize(mRsrc->gameLogo->size().toSizeF());
    dest.setHeight(mBoardRegion.width() * 1.4F / (dest.width() / dest.height()));
    dest.setWidth(mBoardRegion.width() * 1.4F);
    dest.moveTopLeft(mBoardRegion.topLeft() + QPointF((mBoardRegion.width() - dest.width()) / 2, -dest.height() / 1.2F));
    p.drawPixmap(dest, *mRsrc->gameLogo, {});

    // Draw grid & cells (central)
    dest = mBoardRegion;
    p.drawPixmap(dest + QMarginsF(84, 84, 84, 84), *mRsrc->gridBackgroundBorder, {});
    p.drawPixmap(dest, *mRsrc->gridBackgroundBg, {});
    p.drawPixmap(dest, *mRsrc->gridBackground, {});

    destPoint.setX(qFloor(mCellSquare * (mMousePoint.x() - mBoardRegion.x()) / (mBoardRegion.width())));
    destPoint.setY(qFloor(mCellSquare * (mMousePoint.y() - mBoardRegion.y()) / (mBoardRegion.height())));

    for(z = 0; z < mGrid.size(); ++z)
    {
        x = z % mCellSquare;
        y = z / mCellSquare;
        dest.moveLeft(mBoardRegion.x() + x * mScaleFactor.width());
        dest.moveTop(mBoardRegion.y() + y * mScaleFactor.height());
        dest.setSize(mScaleFactor);

        w = mGrid[z] & 0x3;
        if(w == 2)
        {
            p.setOpacity(1.0);
            pixmap = &(*mRsrc->gridCellBright);
        }
        else
        {
            p.setOpacity(0.3);
            pixmap = &(*mRsrc->gridCell);
        }
        p.drawPixmap(dest, *pixmap, {});

        if(w == 1)
        {
            if(mCurrentShape == nullptr && (x == destPoint.x()) && (y == destPoint.y()))
            {
                dest += QMarginsF(3, 3, 3, 3);
                pixmap = getColoredPixmap(mGrid[z] >> 2, mFrameIndex, mRsrc);
                if(mLastSelectedBlock != z)
                {
                    mRsrc->soundManager->playSound("block-hits", 0.3);
                    mLastSelectedBlock = z;
                }
            }
            else
            {
                pixmap = getColoredPixmap(mGrid[z] >> 2, 0, mRsrc);
            }
            p.setOpacity(1.0);
            p.drawPixmap(dest, *pixmap, {});
        }
    }

    p.setOpacity(1.0);

    // Draw blocks
    dest.moveTopLeft(mBoardRegion.topLeft());
    dest.setSize(mScaleFactor);
    destPoint = dest.topLeft();
    y = dest.size().width() / 2;
    for(x = 0; x < mDestroyBlocks.size(); ++x)
    {
        const auto &db = mDestroyBlocks[x];
        dest.moveTopLeft(db.first.adjustPoint(destPoint, dest.size()));
        pixmap = getColoredPixmap(db.second, 0, mRsrc);
        p.drawPixmap(dest.marginsRemoved(QMarginsF(y * (1 - mDestroyScaler), y * (1 - mDestroyScaler), y * (1 - mDestroyScaler), y * (1 - mDestroyScaler))), *pixmap, {});
    }

    // Draw bottom INVENTORY
    dest.moveTopLeft(mBoardRegion.topLeft() + QPointF(0, mBoardRegion.height() + heightOffsetCandidates));
    dest.setSize(mScaleFactor * (static_cast<float>(mCellSquare) / mShapeCandidates.size()));
    if(shapeCandidateIdx != -1 && mShapeCandidates[shapeCandidateIdx])
    {
        // tmpPixmap = std::move(adjustBright(*getColoredPixmap((*shapeCandidates[shapeCandidateIdx]).shapeColor, frameIndex), 60));
        pixmap = getColoredPixmap((*mShapeCandidates[shapeCandidateIdx]).shapeColor, mFrameIndex, mRsrc);
    }

    for(z = 0; z < mShapeCandidates.size(); ++z)
    {
        p.drawPixmap(dest, *mRsrc->gridCell, {});
        if(mShapeCandidates[z])
        {
            destPoint.setX(mScaleFactor.width() * 0.6F * mShapeCandidates[z]->columns);
            destPoint.setY(mScaleFactor.height() * 0.6F * mShapeCandidates[z]->rows);
            destPoint = dest.topLeft() + QPointF((dest.width() - destPoint.x()) / 2, (dest.height() - destPoint.y()) / 2);
            drawShapeAt(*mShapeCandidates[z], destPoint, mScaleFactor * 0.6F, 0, p, mRsrc, ((z == shapeCandidateIdx) ? pixmap : nullptr));
        }
        dest.moveLeft(dest.x() + dest.width());
    }

    // Draw blocks by select pointer
    if(mCurrentShape)
    {
        p.setOpacity(0.8);
        dest.setSize(mScaleFactor * 0.9F);
        destPoint = {mMousePoint.x() - static_cast<float>(mCurrentShape->columns * dest.width()) / 2, mMousePoint.y() - static_cast<float>(mCurrentShape->rows * dest.height()) / 2};
        drawShapeAt(*mCurrentShape, destPoint, dest.size(), mFrameIndex, p, mRsrc);
    }

    // p.drawText(QPoint {10, 200}, QString("Score: ") + QString::number(scores));

    // DRAW TEXT
    if(mDestroyScaler > 0)
    {
        auto font = p.font();
        font.setPixelSize(128 * mDestroyScaler);
        p.setFont(font);
        destPoint.setX(mBoardRegion.x() - 400 * (1 - mDestroyScaler));
        destPoint.setY(mBoardRegion.y() + (mBoardRegion.height() + font.pixelSize()) / 2);
        p.drawText(destPoint, "МОЛОДЕЦ!");
    }
}

QPointF BlockObject::adjustPoint(const QPointF &adjust, const QSizeF &scale) const
{
    QPointF out;
    out.setX((adjust.x() + x * scale.width()));
    out.setY((adjust.y() + y * scale.height()));
    return out;
}
