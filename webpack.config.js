var path = require('path');
var webpack = require('webpack');
const UglifyJsPlugin = require("uglifyjs-webpack-plugin");
const MinifyPlugin = require("babel-minify-webpack-plugin");
const glob = require("glob");

module.exports = {
    devServer: {
        inline: true,
        contentBase: './src',
        port: 3000
    },
    devtool: 'cheap-module-eval-source-map',
    mode: 'development',
    entry: {
        entry: './dev/js/index.js'
    },
    output: {
        path: __dirname + '/src/js',
        publicPath: '/',
        filename: 'bundle.min.js'
    },
    optimization: {
        minimizer: [new UglifyJsPlugin()],
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
        //new MinifyPlugin(minifyOpts, pluginOpts),
        new webpack.optimize.OccurrenceOrderPlugin()
    ],
    module: {
        rules: [
            {
                test: /\.m?js$/,
                exclude: /(node_modules)/,
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
