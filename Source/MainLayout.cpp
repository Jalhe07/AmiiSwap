#include "MainApplication.hpp"

namespace ui
{
    extern MainApplication *mainapp;

    void MainLayout::GetFolders()
    {
    	std::vector<std::string> gameFolders = utils::get_directories("sdmc:/amiibos/");

    	for (auto & element : gameFolders) {
    		std::size_t found = element.find_last_of("/\\");
    		std::string namePath = element.substr(found+1);

    		amiibo::AmiiboGame *game = new amiibo::AmiiboGame(namePath, "sdmc:/amiibos/" + element + "/");

    		std::vector<amiibo::AmiiboFile*> amiiboFiles;

    		utils::getFiles(game->GetPath(), [&game](const std::string &path) {
    			std::size_t found1 = path.find_last_of("/\\");
    			std::string namePath1 = path.substr(found1+1);
    			namePath1.erase(namePath1.length() - 4);

    			amiibo::AmiiboFile *file = new amiibo::AmiiboFile(namePath1, path);

    			game->AddAmiiboFile(file);
    		});

    		this->amiiboGames.push_back(game);
    	}
    }

    std::vector<amiibo::AmiiboFile *> MainLayout::GetEmuiibo()
    {
    	std::vector<amiibo::AmiiboFile *> amiiboFiles;
    	utils::getFiles("sdmc:/emuiibo/", [&amiiboFiles](const std::string &path) {
    		std::size_t found1 = path.find_last_of("/\\");
    		std::string namePath1 = path.substr(found1+1);
    		namePath1.erase(namePath1.length() - 4);
    		amiibo::AmiiboFile *file = new amiibo::AmiiboFile(namePath1, path);

    		amiiboFiles.push_back(file);
    	});
    	return (amiiboFiles);
    }

    MainLayout::MainLayout()
    {
    	if (utils::IsEmuiiboPresent())
    		this->isEmuuibo = true;
    	utils::EnsureDirectories();
    	if (this->isEmuuibo) {
    		this->files = this->GetEmuiibo();
    		this->amiiboMenu = new pu::element::Menu(0, 50, 1280, {255,255,255,255}, 70, 9);
    	} else {
    		this->GetFolders();
    		this->gamesMenu = new pu::element::Menu(0, 50, 1280, {255,255,255,255}, 70, 9);
    		gamesMenu->SetOnFocusColor({102,153,204,255});
    	    this->amiiboMenu = new pu::element::Menu(0, 50, 1280, {255,255,255,255}, 70, 9);
    	}
    	amiiboMenu->SetOnFocusColor({102,153,204,255});
        this->titleText = new pu::element::TextBlock(640, 10, "AmiiSwap");

    	this->titleText->SetTextAlign(pu::element::TextAlign::CenterHorizontal);
    	if (!this->isEmuuibo) {
    		for (auto & element : this->amiiboGames) {
    			pu::element::MenuItem *item = new pu::element::MenuItem(element->GetName());
    			item->AddOnClick(std::bind(&MainLayout::category_Click, this, element), KEY_A);
    			this->gamesMenu->AddItem(item);
    		}
    	} else {
    		for (auto & element : this->files) {
    			pu::element::MenuItem *item = new pu::element::MenuItem(element->GetName());
    			item->AddOnClick(std::bind(&MainLayout::item_Click, this, element), KEY_A);
    			this->amiiboMenu->AddItem(item);
    		}
    	}

        this->AddChild(this->titleText);
    	if (this->isEmuuibo) {
    		this->AddChild(this->amiiboMenu);
    		this->SetElementOnFocus(this->amiiboMenu);
    	} else {
    		this->amiiboMenu->SetVisible(false);
    	    this->AddChild(this->gamesMenu);
    		this->AddChild(this->amiiboMenu);
    		this->SetElementOnFocus(this->gamesMenu);
    	}
    }

    void MainLayout::category_Click(amiibo::AmiiboGame *game)
    {
    	this->waitInput = true;
    	this->amiiboMenu->ClearItems();
    	std::vector<amiibo::AmiiboFile*> files = game->GetBinFiles();
    	for (auto & element : files) {
    		pu::element::MenuItem *item = new pu::element::MenuItem(element->GetName());
    		item->AddOnClick(std::bind(&MainLayout::item_Click, this, element), KEY_A);
    		this->amiiboMenu->AddItem(item);
    	}

    	this->amiiboMenu->SetSelectedIndex(0);
    	this->SetElementOnFocus(this->amiiboMenu);
    	this->amiiboMenu->SetVisible(true);
    	this->gamesMenu->SetVisible(false);
    }

    void MainLayout::item_Click(amiibo::AmiiboFile *element)
    {
    	if (!waitInput) {
    		mainapp->SetWaitBack(true);
    		int sopt = mainapp->CreateShowDialog("Use " + element->GetName() + " ?", "This will set the current Amiibo to " + element->GetName(), { "Yes", "No" }, true);
    		if (sopt == 0) {
    			if (this->isEmuuibo)
    				nfpemuRequestUseCustomAmiibo(element->GetPath().c_str());
    			else
    				utils::copyFile(element->GetPath().c_str(), "sdmc:/amiibo.bin");
    		}
    		mainapp->SetWaitBack(false);
    	} else this->waitInput = false;
    }

    pu::element::Menu *MainLayout::GetGamesMenu()
    {
    	return (this->gamesMenu);
    }

    pu::element::Menu *MainLayout::GetAmiiboMenu()
    {
    	return (this->amiiboMenu);
    }
}