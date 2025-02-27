#include "vin_utils.h"

#include <random>

/**
 * @brief Returns a random VIN string from the predefined list.
 *
 * This function selects a random VIN string using a uniform distribution.
 *
 * @return A random VIN string.
 */
std::string VinUtils::getRandomVinString() {
    const auto& vin_list = getVinList();
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<std::size_t> dist(0, vin_list.size() - 1);
    return vin_list[dist(rng)];
}

/**
 * @brief Returns the predefined list of VINs.
 *
 * This function provides a static list of predefined VIN strings.
 *
 * @return A reference to the list of VINs.
 */
const std::vector<std::string>& VinUtils::getVinList() {
    static const std::vector<std::string> vin_list = {
        "3VWRA7AU7FM024850", "1HGCM82633A004352", "JH4KA8260RC003748",  "2G1FP32P0S2203762",
        "1FTWW33P96ED15457", "4S4BRDLC0B2387119", "1FAFP40461F150129",  "5UXWX9C50D0A14563",
        "JN8AZ2KR5AT250004", "1G1ZT51886F276869", "WDCGG8JBXBF765243",  "3MZBN1W35JM211567",
        "1FTEX1E82FFA51428", "4T1BF1FK3GU523119", "5J8TB4H38GL002345",  "1N4AA6AP3HC429087",
        "WAUVT68E55A136789", "19XFC2F59JE013547", "5YJ3E1EA7JF012345",  "1D7RV1CT8AS121098",
        "1C6RR7JT2GS104567", "1J4PN2GK1BW543890", "5XYPG4A55HG567890",  "1LNHL9FTXDG131222",
        "KM8SRDHF7DU150111", "2HKRM3H53EH345678", "3FADP4EJ9DM200567",  "1C4RJFAG6FC898765",
        "2C3CDXBG6EH199234", "WBA3B1C57FP676567", "1FTEX1EP8GKD912345", "JM3KE2CY3G0798765",
        "5YFBURHE3FP222678", "3C6JR6AT8DG236789"};
    return vin_list;
}