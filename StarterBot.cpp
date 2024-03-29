#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include <Data.h>

StarterBot::StarterBot()
{
    pBT = new BT_DECORATOR("EntryPoint", nullptr);

    BT_PARALLEL_SEQUENCER* pParallelSeq = new BT_PARALLEL_SEQUENCER("MainParallelSequence", pBT, 10);


    //Checking the overlord health
    BT_DECO_REPEATER* pOverlordForeverRepeater = new BT_DECO_REPEATER("RepeatForeverOverlord", pParallelSeq, 0, true, false);


    BT_SELECTOR* pSelectorOverlord = new BT_SELECTOR("OverlordSelector", pOverlordForeverRepeater, 10);

    BT_DECO_CONDITION_HP_OVERLORD* pHpOverlord = new BT_DECO_CONDITION_HP_OVERLORD("HpOverlord", pSelectorOverlord);

    BT_SELECTOR* pSelectorOverlordEnoughHP = new BT_SELECTOR("OverlordSelectorEnoughHP", pHpOverlord, 10);

    BT_DECO_CONDITION_BASE_FOUND* pFoundBase = new BT_DECO_CONDITION_BASE_FOUND("FoundBase", pSelectorOverlordEnoughHP);

    BT_ACTION_ZIGZAG_OVERLORD* pZigZagOverlord = new BT_ACTION_ZIGZAG_OVERLORD("ZigZagOverlord", pFoundBase);

    BT_ACTION_SCOUT_OVERLORD* pScoutOverlord = new BT_ACTION_SCOUT_OVERLORD("ScoutOverlord", pSelectorOverlordEnoughHP);

    BT_ACTION_FLEE_OVERLORD* pFleeOverlord = new BT_ACTION_FLEE_OVERLORD("FleeOverlord", pSelectorOverlord);



    //Farming Minerals forever
    BT_DECO_REPEATER* pFarmingMineralsForeverRepeater = new BT_DECO_REPEATER("RepeatForeverFarmingMinerals", pParallelSeq, 0, true, false);
    BT_DECO_CONDITION_NOT_ENOUGH_WORKERS_FARMING_MINERALS* pNotEnoughWorkersFarmingMinerals = new BT_DECO_CONDITION_NOT_ENOUGH_WORKERS_FARMING_MINERALS("NotEnoughWorkersFarmingMinerals", pFarmingMineralsForeverRepeater);
    BT_ACTION_SEND_IDLE_WORKER_TO_MINERALS* pSendWorkerToMinerals = new BT_ACTION_SEND_IDLE_WORKER_TO_MINERALS("SendWorkerToMinerals", pNotEnoughWorkersFarmingMinerals);

    //Build Additional Supply Provider
    BT_DECO_REPEATER* pBuildSupplyProviderForeverRepeater = new BT_DECO_REPEATER("RepeatForeverBuildSupplyProvider", pParallelSeq, 0, true, false);
    BT_DECO_CONDITION_NOT_ENOUGH_SUPPLY* pNotEnoughSupply = new BT_DECO_CONDITION_NOT_ENOUGH_SUPPLY("NotEnoughSupply", pBuildSupplyProviderForeverRepeater);
    BT_ACTION_BUILD_SUPPLY_PROVIDER* pBuildSupplyProvider = new BT_ACTION_BUILD_SUPPLY_PROVIDER("BuildSupplyProvider", pNotEnoughSupply);



    //Build Order
    BT_SEQUENCER* pBuildOrder = new BT_SEQUENCER("BuildOrderSequence", pParallelSeq, 10);

    BT_SELECTOR* pStep1 = new BT_SELECTOR("Step1Selector", pBuildOrder, 10);
    BT_CONDITION_STEP_1* pConditionStep1 = new BT_CONDITION_STEP_1("Step1Condition", pStep1);
    BT_ACTION_TRAIN_WORKER* pTrainWorker = new BT_ACTION_TRAIN_WORKER("TrainWorker", pStep1);

    BT_SELECTOR* pStep2 = new BT_SELECTOR("Step2Selector", pBuildOrder, 10);
    BT_CONDITION_STEP_2* pConditionStep2 = new BT_CONDITION_STEP_2("Step2Condition", pStep2);
    BT_ACTION_BUILD_POOL* pBuildPool = new BT_ACTION_BUILD_POOL("BuildPool", pStep2);

    BT_SELECTOR* pStep2B = new BT_SELECTOR("Step2BSelector", pBuildOrder, 10);
    BT_CONDITION_STEP_2B* pConditionStep2B = new BT_CONDITION_STEP_2B("Step2BCondition", pStep2B);
    BT_ACTION_TRAIN_WORKER* pTrainWorker2B = new BT_ACTION_TRAIN_WORKER("Train2Workers", pStep2B);

    BT_SELECTOR* pStep3 = new BT_SELECTOR("Step3Selector", pBuildOrder, 10);
    BT_CONDITION_STEP_3* pConditionStep3 = new BT_CONDITION_STEP_3("Step3Condition", pStep3);
    BT_ACTION_TRAIN_ZERGLINGS* pMakeFirstZerglingsArmy = new BT_ACTION_TRAIN_ZERGLINGS("MakeFirstZerglingsArmy", pStep3);



    BT_DECO_REPEATER* pBuildZerglingsRepeater = new BT_DECO_REPEATER("RepeatBuildZerglings", pBuildOrder, 0, true, false);
    BT_ACTION_TRAIN_ZERGLINGS* pRepeatMakeZerglings = new BT_ACTION_TRAIN_ZERGLINGS("RepeatMakeZerglings", pBuildZerglingsRepeater);

    BT_ACTION_SEND_ZERGLINGS* pSendZerglings = new BT_ACTION_SEND_ZERGLINGS("SendZerglings", pParallelSeq);



    pData = new Data();
    pData->currMinerals = 0;
    pData->thresholdMinerals = THRESHOLD1_MINERALS;
    pData->currSupply = 0;
    pData->thresholdSupply = THRESHOLD1_UNUSED_SUPPLY;

    pData->nWantedWorkersTotal = NWANTED_WORKERS_TOTAL;
    pData->nWantedWorkersFarmingMinerals = NWANTED_WORKERS_FARMING_MINERALS;

    pData->step1 = false;
    pData->step3 = false;


    //FIND THE OVERLORD 
    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
    {
        if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
        {
            pData->myOverlord = unit;
            break;
        }
    }

    for (auto& mineral : BWAPI::Broodwar->getMinerals()) {
        pData->unvisitedMinerals.push_back(mineral->getInitialTilePosition());
    }





    //CREATE THE LIST OF VISISTED STARTING POINTS
    pData->visistedStartingPositions = std::vector<BWAPI::TilePosition>();
    pData->visistedStartingPositions.push_back(BWAPI::Broodwar->self()->getStartLocation());
    pData->enemyBase;
    pData->enemyBaseFound = false;
}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
    BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);

    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();

    pData->currMinerals = BWAPI::Broodwar->self()->minerals();
    pData->currSupply = Tools::GetUnusedSupply(true);

    if (pBT != nullptr && pBT->Evaluate(pData) != BT_NODE::RUNNING)
    {
        delete (BT_DECORATOR*)pBT;
    }

    exploreMinerals();


    // if we have enough minerals to build a new hatchery, do so
    bool enoughMineralsHatchery = BWAPI::Broodwar->self()->minerals() >= 350;
    if (enoughMineralsHatchery) {
        BWAPI::UnitType builderType = BWAPI::UnitTypes::Zerg_Hatchery.whatBuilds().first;
        BWAPI::Unit builder = Tools::GetUnitOfType(builderType);
        if (builder) {
            BWAPI::TilePosition startPos = BWAPI::Broodwar->self()->getStartLocation();
            int maxBuildRange = 80;
            bool buildingOnCreep = BWAPI::UnitTypes::Zerg_Hatchery.requiresCreep();
            BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Zerg_Hatchery, startPos, maxBuildRange, buildingOnCreep);
            builder->build(BWAPI::UnitTypes::Zerg_Hatchery, buildPos);
        }
    }

    // If we have 2 hatcheries or more, we want to upgrade metabolic boost
    int numHatcheries = 0;
    for (auto& unit : BWAPI::Broodwar->self()->getUnits()) {
        if (unit->getType() == BWAPI::UnitTypes::Zerg_Hatchery) {
            numHatcheries++;
        }
    }
    bool hatchery = numHatcheries > 1;
    
    // first build an esxtractor
    bool enoughMineralsExtractor = BWAPI::Broodwar->self()->minerals() >= 50 && !(BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Extractor) > 0);
    if (enoughMineralsExtractor && hatchery) {
        BWAPI::UnitType builderType = BWAPI::UnitTypes::Zerg_Extractor.whatBuilds().first;
        BWAPI::Unit builder = Tools::GetUnitOfType(builderType);
        if (builder) {
            // Find a suitable location for the extractor
            BWAPI::TilePosition location = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Zerg_Extractor, builder->getTilePosition());
            builder->build(BWAPI::UnitTypes::Zerg_Extractor, location);
        }
    }

    // Then upgrade metabolic boost
    bool hasMetabolicBoost = BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Metabolic_Boost) > 0;
    bool enoughSpeed = BWAPI::Broodwar->self()->minerals() >= 100 && BWAPI::Broodwar->self()->gas() >= 100 && !hasMetabolicBoost;
    if (enoughSpeed && hatchery) {
        // Find the Spawning Pool
        BWAPI::Unit spawningPool = nullptr;
        BWAPI::Unitset myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits) {
            if (unit->getType() == BWAPI::UnitTypes::Zerg_Spawning_Pool) {
                spawningPool = unit;
                break;
            }
        }
        spawningPool->upgrade(BWAPI::UpgradeTypes::Metabolic_Boost);
    }


    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= 2) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner)
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit destroyedUnit)
{
    //if the unit is farming then remove it from data structure
    if (pData->unitsFarmingMinerals.contains(destroyedUnit)) pData->unitsFarmingMinerals.erase(destroyedUnit);

    // Check if the destroyed unit is an Overlord and whether it is the same as pData->myOverlord
    if (destroyedUnit == pData->myOverlord)
    {
        BWAPI::Broodwar->sendText("The main overlord died");


        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord && unit != pData->myOverlord)
            {
                pData->myOverlord = unit;
                BWAPI::Broodwar->sendText("The main overlord was replaced");

                break;
            }
        }
    }


}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
    // Check if the morphed unit is an Overlord
    if (unit->getType() == BWAPI::UnitTypes::Zerg_Overlord)
    {
        // Send a message in the chat
        BWAPI::Broodwar->sendText("An Overlord has been morphed!");
        if (pData->myOverlord != nullptr) {
            BWAPI::Broodwar->sendText("The overlord was sent exploring!");
            pData->mineralsExploringOverlords.push_back(unit);
        }
        else {
            BWAPI::Broodwar->sendText("Morphed overlord became the main overlord");
            pData->myOverlord = unit;
        }
    }
}

void StarterBot::exploreMinerals()
{
    for (auto& overlord : pData->mineralsExploringOverlords) {
        overlord->move(BWAPI::Position(pData->unvisitedMinerals[0]));
        if (Tools::distance(overlord->getPosition(), BWAPI::Position(pData->unvisitedMinerals[0])) < 50) {
            for (auto& mineral : pData->unvisitedMinerals) {
                if (Tools::distance(overlord->getPosition(), BWAPI::Position(mineral)) < 250) {
                    pData->unvisitedMinerals.erase(pData->unvisitedMinerals.begin());
                }
            }
            break;
        }
    }
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text)
{
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{

}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{

}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{

}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{

}