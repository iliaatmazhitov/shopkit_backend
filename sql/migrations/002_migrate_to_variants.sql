-- Migration: Convert existing products table to new variants system
-- This migration is BACKWARD COMPATIBLE - existing data is preserved

-- Step 1: Rename existing products table
ALTER TABLE IF EXISTS products RENAME TO products_old;

-- Step 2: Create new products table (base products)
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

-- Step 3: Create product_variants table
CREATE TABLE IF NOT EXISTS product_variants (
    id SERIAL PRIMARY KEY,
    variant_id UUID DEFAULT gen_random_uuid() UNIQUE NOT NULL,
    product_id UUID NOT NULL REFERENCES products(product_id) ON DELETE CASCADE,
    sku VARCHAR(100),
    title VARCHAR(255) NOT NULL,
    price INTEGER NOT NULL CHECK (price > 0),
    currency VARCHAR(3) DEFAULT 'RUB',
    image_url TEXT,
    description TEXT,
    stock_count INTEGER DEFAULT 0 CHECK (stock_count >= 0),
    attributes JSONB DEFAULT '{}',
    is_active BOOLEAN DEFAULT TRUE,
    is_default BOOLEAN DEFAULT TRUE,  -- All migrated products will be default variants
    sort_order INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Step 4: Migrate existing products to new structure
-- Each old product becomes a base product + one default variant
INSERT INTO products (id, shop_id, product_id, title, description, category, tags, is_active, created_at, updated_at)
SELECT id, shop_id, product_id, title, description, category, tags, is_active, created_at, updated_at
FROM products_old;

-- Create variants from old products
INSERT INTO product_variants (
    product_id, 
    title, 
    price, 
    currency, 
    image_url, 
    description, 
    stock_count, 
    attributes, 
    is_active,
    is_default,
    created_at,
    updated_at
)
SELECT 
    product_id,
    title,  -- Variant uses same title as product
    price,
    currency,
    image_url,
    description,
    COALESCE(stock_count, 0),
    COALESCE(attributes, '{}'::jsonb),
    is_active,
    TRUE,  -- Mark as default variant
    created_at,
    updated_at
FROM products_old;

-- Step 5: Create indexes
CREATE INDEX IF NOT EXISTS idx_products_shop_id ON products(shop_id);
CREATE INDEX IF NOT EXISTS idx_products_category ON products(category);
CREATE INDEX IF NOT EXISTS idx_variants_product_id ON product_variants(product_id);
CREATE INDEX IF NOT EXISTS idx_variants_attributes ON product_variants USING GIN (attributes);
CREATE INDEX IF NOT EXISTS idx_variants_sku ON product_variants(sku);

-- Step 6: Create view for easy querying
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

-- Step 7: Create triggers for auto-updating updated_at
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = NOW();
    RETURN NEW;
END;
$$ language 'plpgsql';

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

-- Step 8: Update sequence for products table to continue from old max id
SELECT setval('products_id_seq', (SELECT MAX(id) FROM products), true);

-- Step 9: Drop old table (OPTIONAL - comment out if you want to keep backup)
-- DROP TABLE IF EXISTS products_old;

-- Step 10: Add comments
COMMENT ON TABLE products IS 'Base products table - parent products that group variants';
COMMENT ON TABLE product_variants IS 'Product variants - each variant has own price, image, stock, and attributes';

-- Migration complete!
-- All existing products are now in the new structure with one default variant each
