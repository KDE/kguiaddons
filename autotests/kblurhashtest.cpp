// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#include <KBlurHash>

#include <QTest>

class KBlurHashTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void decode83_data();
    void decode83();

    void unpackComponents();

    void decodeMaxAC();

    void decodeAverageColor_data();
    void decodeAverageColor();

    void decodeAC();

    void decodeImage();
};

void KBlurHashTest::decode83_data()
{
    QTest::addColumn<QString>("value");
    QTest::addColumn<std::optional<int>>("expected");

    // invalid base83 characters
    QTest::addRow("decoding 1") << "試し" << std::optional<int>(std::nullopt);
    QTest::addRow("decoding 2") << "(" << std::optional<int>(std::nullopt);

    QTest::addRow("decoding 3") << "0" << std::optional(0);
    QTest::addRow("decoding 4") << "L" << std::optional(21);
    QTest::addRow("decoding 5") << "U" << std::optional(30);
    QTest::addRow("decoding 6") << "Y" << std::optional(34);
    QTest::addRow("decoding 7") << "1" << std::optional(1);
}

void KBlurHashTest::decode83()
{
    QFETCH(QString, value);
    QFETCH(std::optional<int>, expected);

    QCOMPARE(KBlurHash::decode83(value), expected);
}

void KBlurHashTest::unpackComponents()
{
    KBlurHash::Components components;
    components.x = 6;
    components.y = 6;
    QCOMPARE(KBlurHash::unpackComponents(50), components);
}

void KBlurHashTest::decodeMaxAC()
{
    QCOMPARE(KBlurHash::decodeMaxAC(50), 0.307229f);
}

void KBlurHashTest::decodeAverageColor_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<QColor>("expected");

    QTest::addRow("decoding 1") << 12688010 << QColor(0xffc19a8a);
    QTest::addRow("decoding 2") << 9934485 << QColor(0xff979695);
    QTest::addRow("decoding 3") << 8617624 << QColor(0xff837e98);
    QTest::addRow("decoding 4") << 14604757 << QColor(0xffded9d5);
    QTest::addRow("decoding 5") << 13742755 << QColor(0xffd1b2a3);
}

void KBlurHashTest::decodeAverageColor()
{
    QFETCH(int, value);
    QFETCH(QColor, expected);

    QCOMPARE(KBlurHash::decodeAverageColor(value), expected);
}

void KBlurHashTest::decodeAC()
{
    constexpr auto maxAC = 0.289157f;
    QCOMPARE(KBlurHash::decodeAC(0, maxAC), QColor::fromRgbF(-0.289063f, -0.289063f, -0.289063f));
}

void KBlurHashTest::decodeImage()
{
    const auto image = KBlurHash::decode(QStringLiteral("eBB4=;054UK$=402%s%|r^O%06#?*7RijMxGpYMzniVNT@rFN3#=Kt"), QSize(50, 50));
    QVERIFY(!image.isNull());

    QCOMPARE(image.width(), 50);
    QCOMPARE(image.height(), 50);
    QCOMPARE(image.pixelColor(0, 0), QColor(0xff005f00));
    QCOMPARE(image.pixelColor(30, 30), QColor(0xff99b76d));
}

QTEST_GUILESS_MAIN(KBlurHashTest)
#include "kblurhashtest.moc"