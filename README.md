# 🛍️ ShopKit Backend - Production-Ready SaaS Backend

A complete, production-ready backend for a Telegram-based shop platform built with C++ and Drogon framework.

## ✨ Features

### Core Functionality
- ✅ **Shop Management**: Create and manage multiple shops with unique tokens
- ✅ **Product Management**: Full CRUD operations for products with images
- ✅ **Order Management**: Create and track customer orders
- ✅ **File Upload**: Secure image upload with validation (JPEG, PNG, WEBP)
- ✅ **UUID Generation**: Custom C++ UUID v4 generator (no external dependencies)

### Security & Performance
- ✅ **CORS Support**: Full CORS implementation for web clients
- ✅ **Rate Limiting**: 100 requests per minute per IP address
- ✅ **Input Validation**: Comprehensive validation for all endpoints
- ✅ **SQL Injection Prevention**: Parameterized queries throughout

## 🏗️ Architecture

```
shopkit_backend/
├── shopkit/
│   ├── controllers/         # HTTP request handlers
│   │   ├── ShopController.*     # Shop CRUD operations
│   │   ├── ProductController.*  # Product management
│   │   ├── OrderController.*    # Order processing
│   │   └── UploadController.*   # File upload handling
│   ├── filters/             # Request/Response filters
│   │   ├── CorsFilter.*         # CORS headers
│   │   └── RateLimitFilter.*    # Rate limiting
│   ├── utils/               # Utility classes
│   │   └── UuidGenerator.h      # UUID v4 generation
│   └── main.cc              # Application entry point
├── sql/
│   └── schema.sql           # PostgreSQL database schema
└── CMakeLists.txt           # Build configuration
```

## 🚀 Quick Start

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 11+)
- CMake 3.5+
- PostgreSQL 12+
- Drogon Framework

### Installation

1. **Clone the repository**
```bash
git clone https://github.com/iliaatmazhitov/shopkit_backend.git
cd shopkit_backend
```

2. **Install Drogon Framework**
```bash
# Ubuntu/Debian
sudo apt-get install git gcc g++ cmake libjsoncpp-dev uuid-dev \
    openssl libssl-dev zlib1g-dev postgresql-dev

git clone https://github.com/drogonframework/drogon
cd drogon
git submodule update --init
mkdir build && cd build
cmake ..
make && sudo make install
```

3. **Setup Database**
```bash
# Create database
createdb shopkit

# Run schema
psql -d shopkit -f sql/schema.sql
```

4. **Build the project**
```bash
cd shopkit
mkdir build && cd build
cmake ..
make
```

5. **Run the server**
```bash
./shopkit
```

The server will start on `http://0.0.0.0:8080`

## 📡 API Endpoints

### Shop Management

#### Get Shop by Token
```http
GET /api/shops/token/{shop_token}
```

#### Create Shop
```http
POST /api/shops
Content-Type: application/json

{
  "owner_tg_id": 123456789,
  "title": "My Shop",
  "description": "Shop description",
  "currency": "RUB"
}
```

#### Update Shop
```http
PUT /api/shops/{shop_token}
X-Owner-TG-ID: 123456789
Content-Type: application/json

{
  "title": "Updated Title",
  "description": "New description"
}
```

### Product Management

#### Get Products
```http
GET /api/products/{shop_id}
```

#### Get Product Detail
```http
GET /api/products/detail/{product_id}
```

#### Create Product (Admin)
```http
POST /api/admin/products
Content-Type: application/json

{
  "shop_id": 1,
  "title": "Product Name",
  "price": 1000,
  "currency": "RUB",
  "image_url": "/uploads/products/image.jpg",
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
  "price": 1200
}
```

#### Delete Product (Admin)
```http
DELETE /api/admin/products/{product_id}
```

### Order Management

#### Create Order
```http
POST /api/orders
Content-Type: application/json

{
  "shop_id": 1,
  "customer_name": "John Doe",
  "telegram_user_id": 123456789,
  "customer_phone": "+1234567890",
  "items": [
    {
      "product_id": "uuid-here",
      "quantity": 2,
      "price": 1000
    }
  ],
  "total_price": 2000,
  "currency": "RUB",
  "payment_method": "card"
}
```

#### Get Orders (Admin)
```http
GET /api/admin/orders/{shop_id}
```

#### Update Order Status (Admin)
```http
PUT /api/admin/orders/{order_id}
Content-Type: application/json

{
  "status": "completed"
}
```

### File Upload

#### Upload Product Image
```http
POST /api/admin/upload/product-image
Content-Type: multipart/form-data

file: [image file]
```

**Constraints:**
- Maximum file size: 5 MB
- Allowed types: JPEG, PNG, WEBP
- Returns: `{ "success": true, "image_url": "/uploads/products/xxx.jpg" }`

## 🔒 Security Features

### Rate Limiting
- **Limit**: 100 requests per minute per IP
- **Response**: HTTP 429 Too Many Requests
- **Header**: `Retry-After: 60`

### CORS
- Supports all origins (`Access-Control-Allow-Origin: *`)
- Allowed methods: GET, POST, PUT, DELETE, OPTIONS
- Allowed headers: Content-Type, X-Owner-TG-ID

### Authentication
- Shop updates require `X-Owner-TG-ID` header
- Owner validation against database

## 🛠️ Configuration

Edit `shopkit/main.cc` to configure:

```cpp
// Database connection
drogon::app().createDbClient(
    "postgresql",       // Database type
    "127.0.0.1",        // Host
    5432,               // Port
    "shopkit",          // Database name
    "username",         // User
    "password",         // Password
    10,                 // Connection pool size
    "",                 // Filename (unused)
    "default",          // Client name
    false,              // Fast mode
    "utf8"              // Character set
);

// Server configuration
drogon::app()
    .setLogLevel(trantor::Logger::kDebug)
    .addListener("0.0.0.0", 8080)
    .setThreadNum(4)
    .run();
```

## 📊 Database Schema

The project uses PostgreSQL with the following tables:

- **shops**: Store information
- **products**: Product catalog
- **orders**: Customer orders
- **users**: Telegram users
- **shop_admins**: Admin permissions

See `sql/schema.sql` for complete schema definition.

## 🧪 Testing

To test the UUID generator:
```bash
cd shopkit
g++ -std=c++20 -I. test_uuid.cpp -o test_uuid
./test_uuid
```

## 📝 Development Notes

### UUID Generation
The project uses a custom C++ UUID v4 generator to avoid external dependencies:
- No `libuuid` required
- Platform-independent
- Cryptographically random using `std::random_device`
- Format: `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`

### Code Style
- C++20 standard
- Drogon framework conventions
- Async/callback-based request handling
- Parameterized SQL queries

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is private and proprietary.

## 👨‍💻 Author

**Ilya Atmazhitov**
- GitHub: [@iliaatmazhitov](https://github.com/iliaatmazhitov)

## 🙏 Acknowledgments

- [Drogon Framework](https://github.com/drogonframework/drogon) - High-performance C++ HTTP framework
- [PostgreSQL](https://www.postgresql.org/) - Powerful open-source database

---

**Status**: ✅ Production Ready

Last Updated: February 2026
