#include <gtest/gtest.h>
#include "../include/ID.hpp"  // Chemin vers ton header

// Test constructeur avec valeurs explicites
TEST(IDTest, ConstructorExplicit) {
    ID id1(0x123, false);
    EXPECT_EQ(id1.getID(), 0x123);
    EXPECT_FALSE(id1.isExtended());

    ID id2(0x18ABCDEF, true);
    EXPECT_EQ(id2.getID(), 0x18ABCDEF);
    EXPECT_TRUE(id2.isExtended());
}

// Test constructeur avec déduction automatique (is_extended dépend de l'ID)
TEST(IDTest, ConstructorAutoExtended) {
    ID id1(0x123); // inf. à 0x7FF → standard
    EXPECT_FALSE(id1.isExtended());

    ID id2(0x800); // sup. à 0x7FF → extended
    EXPECT_TRUE(id2.isExtended());
}

// Test set()
TEST(IDTest, SetMethod) {
    ID id(0x123, false);
    id.set(0x18FEDF00);
    EXPECT_EQ(id.getID(), 0x18FEDF00);
    EXPECT_TRUE(id.isExtended());

    id.set(0x700);
    EXPECT_EQ(id.getID(), 0x700);
    EXPECT_FALSE(id.isExtended());
}

// Test buildSmartID() (random → on teste juste les propriétés)
TEST(IDTest, BuildSmartID) {
    ID smartId = ID::buildSmartID();
    uint32_t val = smartId.getID();
    if (val <= 0x7FF) {
        EXPECT_FALSE(smartId.isExtended());
    } else {
        EXPECT_TRUE(smartId.isExtended());
    }
}

// Point d’entrée
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
