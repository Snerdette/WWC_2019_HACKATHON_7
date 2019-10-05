var path = require('path');
var webpack = require('webpack');

module.exports = {
    devServer: {
        inline: true,
        contentBase: './src',
        port: 3000
    },
    devtool: 'cheap-module-eval-source-map',
    entry: './dev/js/index.js',
    mode: 'development',
    output: {
        path: '/src/js/bundle.min.js',
        filename: 'bundle.js'
    },
    resolve: {
        extensions:
            [ '*','.js', '.jsx'],
        modules:
            [
                path.join(__dirname, "src"),
                "node_modules"
            ]
    },
    plugins: [
        new webpack.optimize.OccurrenceOrderPlugin()
    ],
    module: {
        rules: [
            {
                test: /\.m?js$/,
                exclude: /(node_modules|bower_components)/,
                use: {
                    loader: 'babel-loader',
                    options: {
                        presets: ['@babel/preset-env']
                    }
                }
            },
            {
                test: /\.scss/,
                loader: 'style-loader!css-loader!sass-loader'
            }
        ]
    }
};
