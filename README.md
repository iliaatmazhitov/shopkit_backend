# ShopKit Backend

A high-performance REST API backend for managing Telegram-based online shops built with [Drogon C++ Framework](https://github.com/drogonframework/drogon) and PostgreSQL.

## 🚀 Features

- **Shop Management**: Create and manage multiple shops with unique tokens
- **Product Management**: Full CRUD operations for products with inventory tracking
- **Order Processing**: Create and track customer orders with flexible status management
- **Database Integration**: PostgreSQL with connection pooling
- **RESTful API**: Clean JSON-based API endpoints
- **High Performance**: Built on Drogon framework for maximum throughput

## 📋 Prerequisites

- **C++ Compiler**: GCC 7+ or Clang 6+ with C++17 support
- **CMake**: Version 3.5 or higher
- **Drogon Framework**: Version 1.0 or higher
- **PostgreSQL**: Version 12 or higher
- **jsoncpp**: For JSON parsing
- **uuid-ossp**: PostgreSQL extension for UUID generation

## 🔧 Installation

### 1. Install Dependencies

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y git gcc g++ cmake libjsoncpp-dev uuid-dev zlib1g-dev \
    postgresql postgresql-contrib libpq-dev
```

#### macOS:
```bash
brew install cmake jsoncpp ossp-uuid postgresql
```

### 2. Install Drogon Framework

```bash
git clone https://github.com/drogonframework/drogon
cd drogon
git submodule update --init
mkdir build && cd build
cmake ..
make -j4
sudo make install
```

### 3. Setup Database

```bash
# Start PostgreSQL
sudo systemctl start postgresql  # Linux
brew services start postgresql   # macOS

# Create database and user
sudo -u postgres psql
CREATE DATABASE shopkit;
CREATE USER ilya WITH PASSWORD '';
GRANT ALL PRIVILEGES ON DATABASE shopkit TO ilya;
\q

# Load schema
psql -U ilya -d shopkit -f sql/schema.sql
```

### 4. Build ShopKit Backend

```bash
cd shopkit
mkdir -p build && cd build
cmake ..
make -j4
```

## 🎯 Running the Server

```bash
cd shopkit/build
./shopkit_backend
```

Expected output:
```
========= ShopKit Backend Starting =========
✅ Database configured: postgresql://ilya@127.0.0.1:5432/shopkit
🚀 Starting HTTP server on http://0.0.0.0:8080
```

## 📚 API Documentation

### Base URL
```
http://localhost:8080
```

### Shop Endpoints

#### Get Shop by Token
```http
GET /api/shops/token/{shop_token}
```

**Response:**
```json
{
  "id": 1,
  "shop_token": "abc123xyz",
  "title": "My Shop",
  "description": "Shop description",
  "currency": "RUB",
  "created_at": "2024-01-01T00:00:00"
}
```

#### Create Shop
```http
POST /api/shops
Content-Type: application/json

{
  "shop_token": "unique_token_here",
  "owner_tg_id": 123456789,
  "title": "My New Shop",
  "description": "Shop description",
  "currency": "RUB"
}
```

#### Update Shop
```http
PUT /api/shops/{shop_token}
Content-Type: application/json

{
  "title": "Updated Shop Name",
  "description": "New description"
}
```

### Product Endpoints

#### Get Shop Products
```http
GET /api/products/{shop_id}
```

**Response:**
```json
[
  {
    "product_id": "uuid-here",
    "title": "Product Name",
    "price": 1000,
    "currency": "RUB",
    "description": "Product description",
    "stock_count": 50,
    "is_active": true
  }
]
```

#### Get Product Details
```http
GET /api/products/detail/{product_id}
```

#### Create Product (Admin)
```http
POST /api/admin/products
Content-Type: application/json

{
  "shop_id": 1,
  "title": "New Product",
  "price": 1000,
  "currency": "RUB",
  "description": "Product description",
  "stock_count": 100,
  "category": "Electronics"
}
```

#### Update Product (Admin)
```http
PUT /api/admin/products/{product_id}
Content-Type: application/json

{
  "title": "Updated Product",
  "price": 1200,
  "stock_count": 75
}
```

#### Delete Product (Admin)
```http
DELETE /api/admin/products/{product_id}
```

### Order Endpoints

#### Create Order
```http
POST /api/orders
Content-Type: application/json

{
  "shop_id": 1,
  "customer_name": "John Doe",
  "customer_phone": "+1234567890",
  "items": [
    {
      "product_id": "uuid-here",
      "title": "Product Name",
      "quantity": 2,
      "price": 1000
    }
  ],
  "total_price": 2000,
  "currency": "RUB",
  "notes": "Please deliver after 6 PM"
}
```

**Response:**
```json
{
  "success": true,
  "order": {
    "order_id": "uuid-here",
    "total_price": 2000,
    "currency": "RUB",
    "status": "pending",
    "created_at": "2024-01-01T12:00:00"
  }
}
```

#### Get Order Details
```http
GET /api/orders/{order_id}
```

**Response:**
```json
{
  "order_id": "uuid-here",
  "shop_id": 1,
  "customer_name": "John Doe",
  "customer_phone": "+1234567890",
  "items": [...],
  "total_price": 2000,
  "currency": "RUB",
  "status": "pending",
  "created_at": "2024-01-01T12:00:00",
  "shop": {
    "title": "Shop Name",
    "shop_token": "abc123"
  }
}
```

#### Get Shop Orders (Admin)
```http
GET /api/admin/orders/{shop_id}
```

**Response:**
```json
[
  {
    "order_id": "uuid-here",
    "customer_name": "John Doe",
    "total_price": 2000,
    "currency": "RUB",
    "status": "pending",
    "created_at": "2024-01-01T12:00:00"
  }
]
```

#### Update Order Status (Admin)
```http
PUT /api/admin/orders/{order_id}
Content-Type: application/json

{
  "status": "completed"
}
```

**Valid Status Values:** `pending`, `processing`, `completed`, `cancelled`

## 🧪 Testing

### Quick API Tests

```bash
# Test shop endpoint
curl http://localhost:8080/api/shops/token/your_shop_token

# Test products
curl http://localhost:8080/api/products/1

# Create an order
curl -X POST http://localhost:8080/api/orders \
  -H "Content-Type: application/json" \
  -d '{
    "shop_id": 1,
    "customer_name": "Test User",
    "items": [{"product_id": "uuid", "quantity": 1, "price": 1000}],
    "total_price": 1000
  }'

# Get order details
curl http://localhost:8080/api/orders/{order_id}
```

### Running Unit Tests

```bash
cd shopkit/build
make test
```

## 🔧 Configuration

Edit `shopkit/config.json` to configure:

- **Database**: Connection settings (host, port, database, user, password)
- **Server**: Listening address and port
- **Logging**: Log level and file locations
- **Threads**: Number of worker threads

Example configuration:
```json
{
  "app": {
    "threads_num": 4,
    "idle_connection_timeout": 60
  },
  "db_clients": [
    {
      "name": "default",
      "rdbms": "postgresql",
      "host": "127.0.0.1",
      "port": 5432,
      "dbname": "shopkit",
      "user": "ilya",
      "passwd": "",
      "connection_number": 10
    }
  ],
  "listeners": [
    {
      "address": "0.0.0.0",
      "port": 8080
    }
  ]
}
```

## 📁 Project Structure

```
shopkit_backend/
├── shopkit/
│   ├── controllers/
│   │   ├── ShopController.h/cc       # Shop management
│   │   ├── ProductController.h/cc    # Product management
│   │   └── OrderController.h/cc      # Order processing
│   ├── models/                       # ORM models (if any)
│   ├── test/                         # Unit tests
│   ├── main.cc                       # Application entry point
│   ├── config.json                   # Configuration file
│   └── CMakeLists.txt                # Build configuration
├── sql/
│   └── schema.sql                    # Database schema
└── README.md                         # This file
```

## 🐛 Troubleshooting

### Database Connection Issues

If you see "Database client is NULL" error:
1. Verify PostgreSQL is running: `sudo systemctl status postgresql`
2. Check database exists: `psql -U ilya -l | grep shopkit`
3. Verify credentials in `config.json`

### Build Errors

If compilation fails:
1. Ensure Drogon is properly installed: `pkg-config --modversion drogon`
2. Check C++17 support: `g++ --version` (should be 7.0+)
3. Verify all dependencies are installed

### Port Already in Use

If port 8080 is busy:
1. Change port in `config.json`
2. Or find and kill the process: `lsof -ti:8080 | xargs kill`

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -am 'Add feature'`
4. Push to the branch: `git push origin feature-name`
5. Submit a pull request

## 📝 License

This project is open source and available under the MIT License.

## 📧 Contact

For questions or support, please open an issue on GitHub.

## 🙏 Acknowledgments

- [Drogon Framework](https://github.com/drogonframework/drogon) - High-performance C++ web framework
- [PostgreSQL](https://www.postgresql.org/) - Powerful open-source database
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) - JSON parsing library
