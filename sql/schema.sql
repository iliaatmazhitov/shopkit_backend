CREATE TABLE IF NOT EXISTS shops (
                                     id SERIAL PRIMARY KEY,
                                     shop_token VARCHAR(64) UNIQUE NOT NULL,
    owner_tg_id BIGINT NOT NULL,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    currency VARCHAR(3) DEFAULT 'RUB',
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
    );

-- Base products table (parent products that group variants)
CREATE TABLE IF NOT EXISTS products (
    id SERIAL PRIMARY KEY,
    shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    product_id UUID DEFAULT gen_random_uuid() UNIQUE,
    title VARCHAR(255) NOT NULL,
    description TEXT,
    category VARCHAR(100),
    tags TEXT[],
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Product variants table (each variant has own price, image, stock, etc.)
CREATE TABLE IF NOT EXISTS product_variants (
    id SERIAL PRIMARY KEY,
    variant_id UUID DEFAULT gen_random_uuid() UNIQUE NOT NULL,
    product_id UUID NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    sku VARCHAR(100),  -- Stock Keeping Unit (optional)
    title VARCHAR(255) NOT NULL,  -- Variant-specific title
    price INTEGER NOT NULL CHECK (price > 0),
    currency VARCHAR(3) DEFAULT 'RUB',
    image_url TEXT,
    description TEXT,  -- Variant-specific description
    stock_count INTEGER DEFAULT 0 CHECK (stock_count >= 0),
    attributes JSONB DEFAULT '{}',  -- e.g. {"storage": "256GB", "color": "Black"}
    is_active BOOLEAN DEFAULT TRUE,
    is_default BOOLEAN DEFAULT FALSE,  -- Mark default variant for display
    sort_order INTEGER DEFAULT 0,  -- For custom ordering
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE IF NOT EXISTS orders (
                                      id SERIAL PRIMARY KEY,
                                      shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    order_id UUID DEFAULT gen_random_uuid() UNIQUE,
    customer_name VARCHAR(255),
    telegram_user_id BIGINT,
    customer_phone VARCHAR(20),
    items JSONB NOT NULL,
    total_price INTEGER NOT NULL,
    currency VARCHAR(3) DEFAULT 'RUB',
    status VARCHAR(50) DEFAULT 'pending',
    payment_method VARCHAR(50),
    notes TEXT,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS users (
                                     id SERIAL PRIMARY KEY,
                                     telegram_user_id BIGINT UNIQUE NOT NULL,
                                     username VARCHAR(255),
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    phone VARCHAR(20),
    created_at TIMESTAMP DEFAULT NOW()
    );

CREATE TABLE IF NOT EXISTS shop_admins (
    shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    telegram_user_id BIGINT NOT NULL,
    role VARCHAR(50) DEFAULT 'manager',
    permissions JSONB DEFAULT '{}',
    created_at TIMESTAMP DEFAULT NOW(),
    PRIMARY KEY (shop_id, telegram_user_id)
    );

-- Indexes for products and variants
CREATE INDEX IF NOT EXISTS idx_products_shop_id ON products(shop_id);
CREATE INDEX IF NOT EXISTS idx_products_category ON products(category);
CREATE INDEX IF NOT EXISTS idx_variants_product_id ON product_variants(product_id);
CREATE INDEX IF NOT EXISTS idx_variants_attributes ON product_variants USING GIN (attributes);
CREATE INDEX IF NOT EXISTS idx_variants_sku ON product_variants(sku);

-- Other indexes
CREATE INDEX IF NOT EXISTS idx_orders_shop_id ON orders(shop_id);
CREATE INDEX IF NOT EXISTS idx_orders_tg_user ON orders(telegram_user_id);
CREATE INDEX IF NOT EXISTS idx_shops_token ON shops(shop_token);
CREATE INDEX IF NOT EXISTS idx_shops_owner ON shops(owner_tg_id);
-- Carts table for user shopping carts
CREATE TABLE IF NOT EXISTS carts (
    id SERIAL PRIMARY KEY,
    telegram_user_id BIGINT NOT NULL,
    shop_id INTEGER NOT NULL REFERENCES shops(id) ON DELETE CASCADE,
    items JSONB NOT NULL,
    updated_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(telegram_user_id, shop_id)
);

CREATE INDEX IF NOT EXISTS idx_carts_user ON carts(telegram_user_id);
CREATE INDEX IF NOT EXISTS idx_carts_shop ON carts(shop_id);

-- View for easy querying of products with their default variant
CREATE OR REPLACE VIEW products_with_default_variant AS
SELECT 
    p.id,
    p.product_id,
    p.shop_id,
    p.title as product_title,
    p.description as product_description,
    p.category,
    p.tags,
    p.is_active as product_active,
    p.created_at as product_created_at,
    p.updated_at as product_updated_at,
    v.variant_id,
    v.sku,
    v.title as variant_title,
    v.price,
    v.currency,
    v.image_url,
    v.description as variant_description,
    v.stock_count,
    v.attributes,
    v.is_active as variant_active,
    v.sort_order
FROM products p
LEFT JOIN LATERAL (
    SELECT *
    FROM product_variants
    WHERE product_id = p.product_id
    ORDER BY is_default DESC, sort_order ASC, created_at ASC
    LIMIT 1
) v ON true;

-- Function to auto-update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Triggers to auto-update updated_at
DROP TRIGGER IF EXISTS update_products_updated_at ON products;
CREATE TRIGGER update_products_updated_at
    BEFORE UPDATE ON products
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

DROP TRIGGER IF EXISTS update_product_variants_updated_at ON product_variants;
CREATE TRIGGER update_product_variants_updated_at
    BEFORE UPDATE ON product_variants
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();


