#include <iostream>
#include <fstream>
#include <ctime>

const int MAX_NAME_LENGTH = 128;
const int MAX_COMMAND_LENGTH = 32;

struct User
{
	int id;
	char name[MAX_NAME_LENGTH];

	User() {}
	User(int id, const char* name)
	{
		this->id = id;
		strcpy_s(this->name, name);
	}
};

struct Wallet
{
	User user;
	double coins;

	Wallet() {}
	Wallet(User user, double coins)
	{
		this->user = user;
		this->coins = coins;
	}
};

struct Transaction
{
	unsigned sender;
	unsigned receiver;
	double coins;
	long long time;

	Transaction() {}
	Transaction(unsigned sender, unsigned receiver, double coins, long long time)
	{
		this->sender = sender;
		this->receiver = receiver;
		this->coins = coins;
		this->time = time;
	}
};

struct TransactionBlock
{
	unsigned id;
	unsigned prevBlockId;
	unsigned prevBlockHash;
	int validTransactions;
	Transaction transactions[16];
};

void expandUsersDB(User*& users, unsigned& numberOfUsers)
{
	numberOfUsers++;
	User* tempDB = new (std::nothrow) User[numberOfUsers];
	if (!tempDB)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	for (size_t i = 0; i < numberOfUsers - 1; i++)
	{
		tempDB[i] = users[i];
	}
	delete[] users;
	users = tempDB;
}

void expandWallets(Wallet*& wallets, unsigned& numberOfUsers)
{
	Wallet* tempWallets = new (std::nothrow) Wallet[numberOfUsers];
	if (!tempWallets)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	for (size_t i = 0; i < numberOfUsers - 1; i++)
	{
		tempWallets[i] = wallets[i];
	}
	delete[] wallets;
	wallets = tempWallets;
}

long long getTime() //wrote this function with the help of chat gpt
{
	// get current time in seconds since 1970
	std::time_t currentTime = std::time(nullptr);

	// get beginning of 1970
	std::tm beginningOf1970 = { 0 };
	beginningOf1970.tm_year = 70;
	beginningOf1970.tm_mday = 1;
	std::time_t beginningOf1970Time = std::mktime(&beginningOf1970);

	// calculate seconds since beginning of 1970
	long long timeSince1970 = static_cast<long long>(currentTime - beginningOf1970Time);

	// print seconds since beginning of 1970
	return timeSince1970;
}

void expandBlocks(TransactionBlock*& blocks, unsigned& numberOfBlocks)
{
	numberOfBlocks++;
	TransactionBlock* temp = new (std::nothrow) TransactionBlock[numberOfBlocks];
	if (!temp)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	for (size_t i = 0; i < numberOfBlocks - 1; i++)
	{
		temp[i] = blocks[i];
	}
	delete[] blocks;
	blocks = temp;
}

unsigned computeHash(const unsigned char* memory, int length)
{
	unsigned hash = 0xbeaf;
	for (int c = 0; c < length; c++)
	{
		hash += memory[c];
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

void transferCoins(User sender, User receiver, double& senderCoins, double& receiverCoins, double coins,
	TransactionBlock*& blocks, unsigned& numberOfBlocks)
{
	if (sender.id != 0 && senderCoins < coins)
	{
		std::cerr << "Not enogh coins!" << std::endl;
		return;
	}
	if (blocks[numberOfBlocks - 1].validTransactions >= 16)
	{
		expandBlocks(blocks, numberOfBlocks);
		blocks[numberOfBlocks - 1].validTransactions = 0;
	}

	blocks[numberOfBlocks - 1].id = numberOfBlocks - 1;
	if (numberOfBlocks == 1)
		blocks[numberOfBlocks - 1].prevBlockId = 0;
	else
		blocks[numberOfBlocks - 1].prevBlockId = numberOfBlocks - 2;
	if (numberOfBlocks == 1)
		blocks[numberOfBlocks - 1].prevBlockHash = 0;
	else
		blocks[numberOfBlocks - 1].prevBlockHash = computeHash(reinterpret_cast<const unsigned char*>(&blocks[numberOfBlocks - 2]), sizeof(TransactionBlock));
	blocks[numberOfBlocks - 1].validTransactions++;
	blocks[numberOfBlocks - 1].transactions[blocks[numberOfBlocks - 1].validTransactions - 1] = Transaction(sender.id, receiver.id, coins, getTime());

	if (sender.id != 0)		senderCoins -= coins;
	if (receiver.id != 0)	receiverCoins += coins;
}

bool checkUserNames(const User* users, unsigned numberOfUsers, const char* name)
{
	for (size_t i = 0; i < numberOfUsers; i++)
	{
		if (strcmp(users[i].name, name) == 0) return true;
	}
	return false;
}

bool isNumber(char* number)
{
	bool decimalPoint = false;
	for (size_t i = 0; number[i] != '\0'; i++)
	{
		if (number[i] != '0' && number[i] != '1' && number[i] != '2' && number[i] != '3' && number[i] != '4' && number[i] != '5'
			&& number[i] != '6' && number[i] != '7' && number[i] != '8' && number[i] != '9' && number[i] != '.')
			return false;
		if (number[i] == '.')
		{
			if (!decimalPoint) decimalPoint = true;
			else return false;
		}
	}
	return true;
}

double getNumberFromString(char* str)
{
	double num = 0;
	double fraction = 1.0;
	size_t i = 0;

	for (; str[i] != '\0' && str[i] != '.'; ++i)
	{
		num = num * 10.0 + (str[i] - '0');
	}

	if (str[i] == '.')
	{
		++i;
		for (; str[i] != '\0'; ++i) {
			fraction /= 10.0;
			num += (str[i] - '0') * fraction;
		}
	}

	return num;
}

void createUser(User*& users, unsigned& numberOfUsers, unsigned& idCounter, Wallet*& wallets,
	TransactionBlock*& blocks, unsigned& numberOfBlocks)
{
	char* name = new (std::nothrow) char[MAX_NAME_LENGTH];
	if (!name)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	std::cout << "Enter user's name: ";
	std::cin.getline(name, MAX_NAME_LENGTH);

	while (checkUserNames(users, numberOfUsers, name))
	{
		std::cerr << "Error: A user with the name \"" << name << "\" already exist!" << std::endl;

		std::cout << "Enter another name for the user: ";
		std::cin.getline(name, MAX_NAME_LENGTH);
	}

	expandUsersDB(users, numberOfUsers);
	expandWallets(wallets, numberOfUsers);
	idCounter++;

	int id = idCounter;
	double coinValueBGN = 0;

	users[numberOfUsers - 1] = User(id, name);
	wallets[numberOfUsers - 1] = Wallet(users[numberOfUsers - 1], 0);

	std::cout << "Enter OOPCoin portfolio value (in BGN): ";
	char number[MAX_NAME_LENGTH];
	std::cin.getline(number, MAX_NAME_LENGTH);
	while (!isNumber(number))
	{
		std::cerr << "This was not a number. Enter OOPCoin portfolio value (in BGN): ";
		std::cin.getline(number, MAX_NAME_LENGTH);
	}
	coinValueBGN = getNumberFromString(number);

	transferCoins(users[0], users[numberOfUsers - 1], wallets[0].coins, wallets[numberOfUsers - 1].coins, coinValueBGN * 420, blocks, numberOfBlocks);
}

void extractUsersFromFile(User*& users, Wallet*& wallets, unsigned& numberOfUsers, unsigned& idCounter, std::ifstream& usersFileReader)
{
	char a;
	usersFileReader.read(reinterpret_cast<char*>(&a), sizeof(a));
	if (usersFileReader.eof())
	{
		usersFileReader.seekg(0, std::ios::beg);
		return;
	}

	usersFileReader.seekg(0, std::ios::beg);

	for (size_t i = 1; !usersFileReader.eof(); i++)
	{
		expandUsersDB(users, numberOfUsers);
		expandWallets(wallets, numberOfUsers);
		User currentUser;
		usersFileReader.read(reinterpret_cast<char*>(&currentUser), sizeof(currentUser));
		Wallet currentWallet(currentUser, 0);
		users[i] = currentUser;
		wallets[i] = currentWallet;

		if (currentUser.id > idCounter) idCounter = currentUser.id;

		usersFileReader.read(reinterpret_cast<char*>(&a), sizeof(a));
		if (usersFileReader.eof())
		{
			return;
		}
		else
		{
			usersFileReader.seekg(-1, std::ios::cur);
		}
	}
}

void uploadUsersToFile(const User* users, const unsigned& numberOfUsers, std::ofstream& usersFileWriter)
{
	for (size_t i = 1; i < numberOfUsers; i++)
	{
		User currentUser = users[i];
		usersFileWriter.write(reinterpret_cast<char*>(&currentUser), sizeof(currentUser)); //<< std::endl;
	}
}

bool checkUserID(const User*& users, unsigned numberOfUsers, const unsigned id)
{
	for (size_t i = 0; i < numberOfUsers; i++)
	{
		if (users[i].id == id) return true;
	}
	return false;
}

void fillWallet(const TransactionBlock* blocks, unsigned& numberOfBlocks, Wallet*& wallets, const User* users, unsigned numberOfUsers)
{
	for (size_t i = 0; i < numberOfBlocks - 1; i++)
	{
		for (size_t j = 0; j < 16; j++)
		{
			if (blocks[i].transactions[j].sender != 0 &&
				checkUserID(users, numberOfUsers, blocks[i].transactions[j].sender))
			{
				for (size_t r = 0; r < numberOfUsers; r++)
					if (wallets[r].user.id == blocks[i].transactions[j].sender)
						wallets[r].coins -= blocks[i].transactions[j].coins;
			}

			if (blocks[i].transactions[j].receiver != 0
				&& checkUserID(users, numberOfUsers, blocks[i].transactions[j].receiver))
			{
				for (size_t r = 0; r < numberOfUsers; r++)
					if (wallets[r].user.id == blocks[i].transactions[j].receiver)
						wallets[r].coins += blocks[i].transactions[j].coins;
			}
		}
	}

	for (int j = 0; j < blocks[numberOfBlocks - 1].validTransactions; j++)    //last block's transactions
	{
		if (blocks[numberOfBlocks - 1].transactions[j].sender != 0
			&& checkUserID(users, numberOfUsers, blocks[numberOfBlocks - 1].transactions[j].sender))
		{
			for (size_t r = 0; r < numberOfUsers; r++)
				if (wallets[r].user.id == blocks[numberOfBlocks - 1].transactions[j].sender)
					wallets[r].coins -= blocks[numberOfBlocks - 1].transactions[j].coins;
		}

		if (blocks[numberOfBlocks - 1].transactions[j].receiver != 0
			&& checkUserID(users, numberOfUsers, blocks[numberOfBlocks - 1].transactions[j].receiver))
		{
			for (size_t r = 0; r < numberOfUsers; r++)
				if (wallets[r].user.id == blocks[numberOfBlocks - 1].transactions[j].receiver)
					wallets[r].coins += blocks[numberOfBlocks - 1].transactions[j].coins;
		}
	}
}

void extractBlocksFromFile(TransactionBlock*& blocks, unsigned& numberOfBlocks, std::ifstream& blocksFileReader)
{
	char a;
	blocksFileReader.read(reinterpret_cast<char*>(&a), sizeof(a));
	if (blocksFileReader.eof())
	{
		blocksFileReader.seekg(0, std::ios::beg);
		return;
	}

	blocksFileReader.seekg(0, std::ios::beg);

	TransactionBlock currentBlock;
	size_t i = 0;

	for (i = 0; !blocksFileReader.eof(); i++)
	{
		expandBlocks(blocks, numberOfBlocks);

		blocksFileReader.read(reinterpret_cast<char*>(&currentBlock), sizeof(currentBlock));
		blocks[i] = currentBlock;
		blocksFileReader.read(reinterpret_cast<char*>(&a), sizeof(a));
		if (blocksFileReader.eof())
		{
			return;
		}
		else
		{
			blocksFileReader.seekg(-1, std::ios::cur);
		}
	}
}

void uploadBlocksToFile(const TransactionBlock* blocks, const unsigned& numberOfBlocks, std::ofstream& blocksFileWriter)
{
	for (size_t i = 0; i < numberOfBlocks; i++)
	{
		TransactionBlock currentBlock = blocks[i];
		blocksFileWriter.write(reinterpret_cast<char*>(&currentBlock), sizeof(currentBlock)); //<< std::endl;
	}
}

bool validUser(const User* users, unsigned& numberOfUsers, const char* usersName)
{
	for (size_t i = 1; i < numberOfUsers; i++)
	{
		if (strcmp(usersName, users[i].name) == 0)
			return true;
	}
	return false;
}

void removeUser(User*& users, Wallet*& wallets, unsigned& numberOfUsers, const char* usersName,
	TransactionBlock*& blocks, unsigned& numberOfBlocks)
{
	if (!validUser(users, numberOfUsers, usersName))
	{
		std::cerr << "User not found!" << std::endl;
		return;
	}

	numberOfUsers--;
	User* tempDB = new (std::nothrow) User[numberOfUsers];
	if (!tempDB)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	Wallet* tempWallets = new (std::nothrow) Wallet[numberOfUsers];
	if (!tempWallets)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}

	bool encounteredUser = false;

	for (size_t i = 0; i < numberOfUsers + 1; i++)
	{
		if (strcmp(usersName, users[i].name) == 0)
		{
			encounteredUser = true;
			transferCoins(users[i], users[0], wallets[i].coins, wallets[0].coins, wallets[i].coins, blocks, numberOfBlocks);
		}

		if (!encounteredUser)
		{
			tempDB[i] = users[i];
			tempWallets[i] = wallets[i];
		}
		else if (i < numberOfUsers)
		{
			tempDB[i] = users[i + 1];
			tempWallets[i] = wallets[i + 1];
		}
	}

	delete[] users;
	users = tempDB;
	delete[] wallets;
	wallets = tempWallets;
}

void sendCoins(const char* usersNameSender, const char* usersNameReceiver, const User* users, Wallet*& wallets, const unsigned numberOfUsers, const double coins,
	TransactionBlock*& blocks, unsigned& numberOfBlocks)
{
	User sender;
	Wallet* sendersWallet = &wallets[0];
	User receiver;
	Wallet* receiversWallet = &wallets[0];

	bool senderFound = false;
	bool receiverFound = false;

	for (size_t i = 1; i < numberOfUsers; i++)
	{
		if (strcmp(users[i].name, usersNameSender) == 0)
		{
			sender = users[i];
			sendersWallet = &wallets[i];
			senderFound = true;
		}
		if (strcmp(users[i].name, usersNameReceiver) == 0)
		{
			receiver = users[i];
			receiversWallet = &wallets[i];
			receiverFound = true;
		}
	}

	if (senderFound && receiverFound)
		transferCoins(sender, receiver, (*sendersWallet).coins, (*receiversWallet).coins, coins, blocks, numberOfBlocks);
	else
	{
		if (!senderFound && !receiverFound) std::cerr << "Error: Sender and receiver not found!" << std::endl;
		else if (!senderFound && receiverFound) std::cerr << "Error: Sender not found!" << std::endl;
		else if (senderFound && !receiverFound) std::cerr << "Error: Receiver not found!" << std::endl;
	}
}

unsigned getNumLenght(long long num)
{
	unsigned counter = 0;
	while (num != 0)
	{
		num /= 10;
		counter++;
	}
	return counter;
}

char* numToStr(long long num)
{
	unsigned numLenght = 0;
	numLenght = getNumLenght(num);
	char* str = new (std::nothrow) char[numLenght + 1];
	if (!str)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	str[numLenght] = '\0';

	for (int i = numLenght - 1; i >= 0; i--)
	{
		str[i] = (num % 10) + '0';
		num /= 10;
	}
	return str;
}

void wealthiestUsers(const Wallet* wallets, unsigned numberOfUsers, unsigned number)
{
	Wallet* walletsTemp = new (std::nothrow) Wallet[numberOfUsers - 1];//creating a copy of the original array without the System User
	if (!walletsTemp)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	for (size_t i = 0; i < numberOfUsers - 1; i++)
	{
		walletsTemp[i] = wallets[i + 1];
	}

	for (size_t i = 0; i < numberOfUsers - 2; i++)//sorting the copy of the original array
	{
		for (size_t j = i + 1; j < numberOfUsers - 1; j++)
		{
			if (walletsTemp[i].coins < walletsTemp[j].coins)
			{
				Wallet temp = walletsTemp[i];
				walletsTemp[i] = walletsTemp[j];
				walletsTemp[j] = temp;
			}
		}
	}

	long long time = getTime();
	char fileName[MAX_NAME_LENGTH] = "wealthiest-users";
	char* str = numToStr(time);
	strcat_s(fileName, str);
	strcat_s(fileName, ".txt");
	delete[] str;

	std::ofstream wealthiestUserWriter(fileName);

	for (size_t i = 0; i < number; i++)
	{
		wealthiestUserWriter << i + 1 << ": " << walletsTemp[i].user.name << " - " << walletsTemp[i].coins << std::endl;
		std::cout << i + 1 << ": " << walletsTemp[i].user.name << " - " << walletsTemp[i].coins << std::endl;
	}

	delete[] walletsTemp;
	wealthiestUserWriter.close();
}

double getCoinsPerBlock(const Transaction* transactions, bool lastBlock, unsigned transactionsInCurrBlock)
{
	double sum = 0;

	int numberOfTransactions = 16;
	if (lastBlock) numberOfTransactions = transactionsInCurrBlock;

	for (size_t i = 0; i < numberOfTransactions; i++)
	{
		sum += transactions[i].coins;
	}
	return sum;
}

void biggestBlocks(const TransactionBlock* blocks, unsigned numberOfBlocks, unsigned number)
{
	TransactionBlock* blocksTemp = new (std::nothrow) TransactionBlock[numberOfBlocks];//creating a copy of the original array so i can sort it
	if (!blocksTemp)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}
	double* coinsPerBlock = new (std::nothrow) double[numberOfBlocks];
	if (!coinsPerBlock)
	{
		std::cerr << "Memory problem!";
		exit(1);
	}

	for (size_t i = 0; i < numberOfBlocks; i++)
	{
		blocksTemp[i] = blocks[i];
		coinsPerBlock[i] = getCoinsPerBlock(blocks[i].transactions, i + 1 == numberOfBlocks, blocks[numberOfBlocks - 1].validTransactions);
	}

	for (int i = 0; i < numberOfBlocks - 1; i++) //sorting
	{
		for (int j = i + 1; j < numberOfBlocks; j++)
		{
			if (coinsPerBlock[i] < coinsPerBlock[j])
			{
				double tempCoins = coinsPerBlock[i];
				coinsPerBlock[i] = coinsPerBlock[j];
				coinsPerBlock[j] = tempCoins;

				TransactionBlock tempBlock = blocksTemp[i];
				blocksTemp[i] = blocksTemp[j];
				blocksTemp[j] = tempBlock;
			}
		}
	}

	long long time = getTime();
	char fileName[MAX_NAME_LENGTH] = "biggest-blocks";
	char* str = numToStr(time);
	strcat_s(fileName, str);
	strcat_s(fileName, ".txt");
	delete[] str;

	std::ofstream biggestBlocksWriter(fileName);

	for (size_t i = 0; i < number; i++)
	{
		biggestBlocksWriter << i + 1 << ": " << blocksTemp[i].id + 1 << " - " << coinsPerBlock[i] << std::endl;
		std::cout << i + 1 << ": " << blocksTemp[i].id + 1 << " - " << coinsPerBlock[i] << std::endl;
	}

	delete[] blocksTemp;
	delete[] coinsPerBlock;
	biggestBlocksWriter.close();
}

bool verifyTransaction(const TransactionBlock* blocks, unsigned numberOfBlocks)
{
	for (size_t i = 0; i < numberOfBlocks - 1; i++)
	{
		if (blocks[i].id != blocks[i + 1].prevBlockId ||
			computeHash(reinterpret_cast<const unsigned char*>(&blocks[i]), sizeof(TransactionBlock)) != blocks[i + 1].prevBlockHash)
			return false;
	}
	return true;
}

int main()
{
	unsigned numberOfUsers = 1;//initializing the System User
	unsigned idCounter = 0;//with this variable i avoid repeating id-s in case of creating new users after deleting others
	User* users = new (std::nothrow) User[numberOfUsers];
	if (!users)
	{
		std::cerr << "Memory problem!";
		return 1;
	}
	users[0] = User(0, "System User");

	Wallet* wallets = new (std::nothrow) Wallet[numberOfUsers];//wallets - every user have a wallet, in which their coins are being saved
	if (!wallets)
	{
		std::cerr << "Memory problem!";
		return 1;
	}
	wallets[0] = Wallet(users[0], 0);

	std::ifstream usersFileReader("users.dat", std::ios::binary);//streams
	if (usersFileReader.is_open())
	{
		extractUsersFromFile(users, wallets, numberOfUsers, idCounter, usersFileReader);
		usersFileReader.close();
	}

	unsigned numberOfBlocks = 0;
	TransactionBlock* blocks = new (std::nothrow) TransactionBlock[numberOfBlocks];
	if (!blocks)
	{
		std::cerr << "Memory problem!";
		return 1;
	}

	std::ifstream blocksFileReader("blocks.dat", std::ios::binary);
	if (blocksFileReader.is_open())
	{
		extractBlocksFromFile(blocks, numberOfBlocks, blocksFileReader);
		blocksFileReader.close();
	}
	else
	{
		expandBlocks(blocks, numberOfBlocks);
		blocks[0].id = 0;
		blocks[0].validTransactions = 0;
	}

	fillWallet(blocks, numberOfBlocks, wallets, users, numberOfUsers);

	char command[MAX_COMMAND_LENGTH];//commands
	char usersName[MAX_NAME_LENGTH];
	char usersName2[MAX_NAME_LENGTH];
	double coins;
	unsigned number;

	std::cout << "Enter command: ";
	std::cin >> command;
	std::cin.ignore();

	while (strcmp(command, "exit") != 0)
	{
		if (strcmp(command, "create-user") == 0)
		{
			createUser(users, numberOfUsers, idCounter, wallets, blocks, numberOfBlocks);
		}
		else if (strcmp(command, "remove-user") == 0)
		{
			std::cout << "Enter user's name: ";
			std::cin.getline(usersName, MAX_NAME_LENGTH);
			removeUser(users, wallets, numberOfUsers, usersName, blocks, numberOfBlocks);
		}
		else if (strcmp(command, "send-coins") == 0)
		{
			std::cout << "From (sender's name): ";
			std::cin.getline(usersName, MAX_NAME_LENGTH);
			std::cout << "To (receiver's name): ";
			std::cin.getline(usersName2, MAX_NAME_LENGTH);
			std::cout << "Enter the amount of coins you would like to transfer (in BGN): ";
			char number[MAX_NAME_LENGTH];
			std::cin.getline(number, MAX_NAME_LENGTH);
			while (!isNumber(number))
			{
				std::cerr << "This was not a number. Enter the amount of coins you would like to transfer (in BGN): ";
				std::cin.getline(number, MAX_NAME_LENGTH);
			}
			coins = getNumberFromString(number);

			sendCoins(usersName, usersName2, users, wallets, numberOfUsers, coins * 420, blocks, numberOfBlocks);
		}
		else if (strcmp(command, "wealthiest-users") == 0)
		{
			std::cout << "Number of users: ";
			std::cin >> number;
			while (number >= numberOfUsers)
			{
				std::cout << "There are not that many users; Enter another number: ";
				std::cin >> number;
			}
			wealthiestUsers(wallets, numberOfUsers, number);
		}
		else if (strcmp(command, "biggest-blocks") == 0)
		{
			std::cout << "Number of blocks: ";
			std::cin >> number;
			while (number > numberOfBlocks)
			{
				std::cout << "There are not that many blocks; Enter another number: ";
				std::cin >> number;
			}
			biggestBlocks(blocks, numberOfBlocks, number);
		}
		else if (strcmp(command, "verify-transactions") == 0)
		{
			if (verifyTransaction(blocks, numberOfBlocks))
				std::cout << "The transaction blocks are valid!" << std::endl;
			else std::cout << "The transaction blocks are not valid!" << std::endl;
		}
		else
		{
			std::cerr << "Invalid command!" << std::endl;
		}

		std::cout << std::endl;
		std::cout << "Enter command: ";
		std::cin >> command;
		std::cin.ignore();
	}

	//when the command is exit:
	std::ofstream usersFileWriter("users.dat", std::ios::binary);
	std::ofstream blocksFileWriter("blocks.dat", std::ios::binary);

	uploadUsersToFile(users, numberOfUsers, usersFileWriter);
	uploadBlocksToFile(blocks, numberOfBlocks, blocksFileWriter);

	usersFileWriter.close();
	blocksFileWriter.close();
	delete[] users;
	delete[] wallets;
	delete[] blocks;
}